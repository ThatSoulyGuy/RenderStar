#include <gtest/gtest.h>
#include "RenderStar/Common/Network/Packets/SceneDataPacket.hpp"

using namespace RenderStar::Common::Network;
using namespace RenderStar::Common::Network::Packets;
using namespace RenderStar::Common::Scene;

namespace
{
    SceneDataPacket RoundTrip(const SceneDataPacket& original)
    {
        auto buffer = PacketBuffer::Allocate();
        original.Write(buffer);
        buffer.Reset();
        SceneDataPacket result;
        result.Read(buffer);
        return result;
    }
}

TEST(SceneDataPacketTest, EmptyPacketRoundTrip)
{
    SceneDataPacket packet;
    auto result = RoundTrip(packet);
    EXPECT_TRUE(result.textures.empty());
    EXPECT_TRUE(result.groups.empty());
}

TEST(SceneDataPacketTest, SingleTextureRoundTrip)
{
    SceneDataPacket packet;

    MapbinTexture tex;
    tex.materialId = 5;
    tex.width = 2;
    tex.height = 2;
    tex.wrapS = 0x2901;
    tex.wrapT = 0x812F;
    tex.minFilter = 0x2601;
    tex.magFilter = 0x2600;
    tex.pixelData = {0xFF, 0x00, 0x80, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
                     0x80, 0x80, 0x80, 0xFF, 0x00, 0x00, 0xFF, 0xFF};
    packet.textures.push_back(tex);

    auto result = RoundTrip(packet);
    ASSERT_EQ(result.textures.size(), 1u);

    const auto& rt = result.textures[0];
    EXPECT_EQ(rt.materialId, 5);
    EXPECT_EQ(rt.width, 2u);
    EXPECT_EQ(rt.height, 2u);
    EXPECT_EQ(rt.wrapS, 0x2901u);
    EXPECT_EQ(rt.wrapT, 0x812Fu);
    EXPECT_EQ(rt.minFilter, 0x2601u);
    EXPECT_EQ(rt.magFilter, 0x2600u);
    ASSERT_EQ(rt.pixelData.size(), 16u);
    EXPECT_EQ(rt.pixelData[0], 0xFF);
    EXPECT_EQ(rt.pixelData[4], 0x00);
}

TEST(SceneDataPacketTest, MultipleTexturesRoundTrip)
{
    SceneDataPacket packet;

    MapbinTexture tex1;
    tex1.materialId = 1;
    tex1.width = 1;
    tex1.height = 1;
    tex1.pixelData = {0xAA, 0xBB, 0xCC, 0xDD};
    packet.textures.push_back(tex1);

    MapbinTexture tex2;
    tex2.materialId = 2;
    tex2.width = 4;
    tex2.height = 4;
    tex2.wrapS = 0x812F;
    tex2.pixelData.resize(64, 0x42);
    packet.textures.push_back(tex2);

    auto result = RoundTrip(packet);
    ASSERT_EQ(result.textures.size(), 2u);
    EXPECT_EQ(result.textures[0].materialId, 1);
    EXPECT_EQ(result.textures[0].pixelData.size(), 4u);
    EXPECT_EQ(result.textures[1].materialId, 2);
    EXPECT_EQ(result.textures[1].width, 4u);
    EXPECT_EQ(result.textures[1].wrapS, 0x812Fu);
    EXPECT_EQ(result.textures[1].pixelData.size(), 64u);
}

TEST(SceneDataPacketTest, SingleGroupRoundTrip)
{
    SceneDataPacket packet;

    MapbinGroup group;
    group.materialId = 7;
    group.vertexCount = 3;
    group.vertexData = {1.0f, 2.0f, 3.0f, 0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
                        4.0f, 5.0f, 6.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
                        7.0f, 8.0f, 9.0f, 0.5f, 0.5f, 0.5f, 0.0f, 1.0f};
    group.indices = {0, 1, 2};
    packet.groups.push_back(group);

    auto result = RoundTrip(packet);
    ASSERT_EQ(result.groups.size(), 1u);

    const auto& rg = result.groups[0];
    EXPECT_EQ(rg.materialId, 7);
    EXPECT_EQ(rg.vertexCount, 3);
    ASSERT_EQ(rg.vertexData.size(), 24u);
    EXPECT_FLOAT_EQ(rg.vertexData[0], 1.0f);
    EXPECT_FLOAT_EQ(rg.vertexData[8], 4.0f);
    EXPECT_FLOAT_EQ(rg.vertexData[16], 7.0f);
    ASSERT_EQ(rg.indices.size(), 3u);
    EXPECT_EQ(rg.indices[0], 0u);
    EXPECT_EQ(rg.indices[1], 1u);
    EXPECT_EQ(rg.indices[2], 2u);
}

TEST(SceneDataPacketTest, TexturesAndGroupsCombined)
{
    SceneDataPacket packet;

    MapbinTexture tex;
    tex.materialId = 10;
    tex.width = 1;
    tex.height = 1;
    tex.pixelData = {0xFF, 0xFF, 0xFF, 0xFF};
    packet.textures.push_back(tex);

    MapbinGroup group;
    group.materialId = 10;
    group.vertexCount = 3;
    group.vertexData.resize(24, 0.5f);
    group.indices = {0, 2, 1};
    packet.groups.push_back(group);

    auto result = RoundTrip(packet);
    ASSERT_EQ(result.textures.size(), 1u);
    ASSERT_EQ(result.groups.size(), 1u);
    EXPECT_EQ(result.textures[0].materialId, 10);
    EXPECT_EQ(result.groups[0].materialId, 10);
}

TEST(SceneDataPacketTest, GroupMaterialIdPreserved)
{
    SceneDataPacket packet;

    MapbinGroup g1;
    g1.materialId = -1;
    g1.vertexCount = 0;
    packet.groups.push_back(g1);

    MapbinGroup g2;
    g2.materialId = 999;
    g2.vertexCount = 0;
    packet.groups.push_back(g2);

    auto result = RoundTrip(packet);
    ASSERT_EQ(result.groups.size(), 2u);
    EXPECT_EQ(result.groups[0].materialId, -1);
    EXPECT_EQ(result.groups[1].materialId, 999);
}

TEST(SceneDataPacketTest, LargeTexturePixelData)
{
    SceneDataPacket packet;

    MapbinTexture tex;
    tex.materialId = 0;
    tex.width = 32;
    tex.height = 32;
    tex.pixelData.resize(32 * 32 * 4);

    for (size_t i = 0; i < tex.pixelData.size(); ++i)
        tex.pixelData[i] = static_cast<uint8_t>(i & 0xFF);

    packet.textures.push_back(tex);

    auto result = RoundTrip(packet);
    ASSERT_EQ(result.textures.size(), 1u);
    ASSERT_EQ(result.textures[0].pixelData.size(), 32u * 32u * 4u);

    for (size_t i = 0; i < result.textures[0].pixelData.size(); ++i)
        EXPECT_EQ(result.textures[0].pixelData[i], static_cast<uint8_t>(i & 0xFF));
}

TEST(SceneDataPacketTest, TextureAllFieldsPreserved)
{
    SceneDataPacket packet;

    MapbinTexture tex;
    tex.materialId = 42;
    tex.width = 128;
    tex.height = 64;
    tex.wrapS = 0x812F;
    tex.wrapT = 0x2901;
    tex.minFilter = 0x2600;
    tex.magFilter = 0x2601;
    tex.pixelData = {1, 2, 3, 4};
    packet.textures.push_back(tex);

    auto result = RoundTrip(packet);
    ASSERT_EQ(result.textures.size(), 1u);
    const auto& rt = result.textures[0];
    EXPECT_EQ(rt.materialId, 42);
    EXPECT_EQ(rt.width, 128u);
    EXPECT_EQ(rt.height, 64u);
    EXPECT_EQ(rt.wrapS, 0x812Fu);
    EXPECT_EQ(rt.wrapT, 0x2901u);
    EXPECT_EQ(rt.minFilter, 0x2600u);
    EXPECT_EQ(rt.magFilter, 0x2601u);
}

TEST(SceneDataPacketTest, EmptyPixelData)
{
    SceneDataPacket packet;

    MapbinTexture tex;
    tex.materialId = 0;
    tex.width = 0;
    tex.height = 0;
    packet.textures.push_back(tex);

    auto result = RoundTrip(packet);
    ASSERT_EQ(result.textures.size(), 1u);
    EXPECT_TRUE(result.textures[0].pixelData.empty());
}

TEST(SceneDataPacketTest, MultipleGroupsVertexDataIntegrity)
{
    SceneDataPacket packet;

    for (int i = 0; i < 3; ++i)
    {
        MapbinGroup group;
        group.materialId = i;
        group.vertexCount = 1;
        group.vertexData = {
            static_cast<float>(i * 10), static_cast<float>(i * 10 + 1),
            static_cast<float>(i * 10 + 2), 0.5f, 0.5f, 0.5f, 0.0f, 0.0f
        };
        group.indices = {};
        packet.groups.push_back(group);
    }

    auto result = RoundTrip(packet);
    ASSERT_EQ(result.groups.size(), 3u);

    for (int i = 0; i < 3; ++i)
    {
        EXPECT_EQ(result.groups[i].materialId, i);
        EXPECT_FLOAT_EQ(result.groups[i].vertexData[0], static_cast<float>(i * 10));
        EXPECT_FLOAT_EQ(result.groups[i].vertexData[1], static_cast<float>(i * 10 + 1));
        EXPECT_FLOAT_EQ(result.groups[i].vertexData[2], static_cast<float>(i * 10 + 2));
    }
}
