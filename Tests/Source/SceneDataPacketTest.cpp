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
    EXPECT_TRUE(result.materials.empty());
    EXPECT_TRUE(result.groups.empty());
}

TEST(SceneDataPacketTest, SingleMaterialRoundTrip)
{
    SceneDataPacket packet;

    MapbinMaterial mat;
    mat.materialId = 5;
    mat.roughness = 0.3f;
    mat.metallic = 0.8f;
    mat.specularStrength = 0.7f;
    mat.normalStrength = 0.9f;
    mat.detailScale = 2.0f;
    mat.emissionStrength = 0.5f;
    mat.aoStrength = 0.6f;

    MapbinTextureSlot slot;
    slot.slotType = TextureSlotType::BASE_COLOR;
    slot.width = 2;
    slot.height = 2;
    slot.wrapS = 0x2901;
    slot.wrapT = 0x812F;
    slot.minFilter = 0x2601;
    slot.magFilter = 0x2600;
    slot.pixelData = {0xFF, 0x00, 0x80, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
                      0x80, 0x80, 0x80, 0xFF, 0x00, 0x00, 0xFF, 0xFF};
    mat.textureSlots.push_back(slot);
    packet.materials.push_back(mat);

    auto result = RoundTrip(packet);
    ASSERT_EQ(result.materials.size(), 1u);

    const auto& rm = result.materials[0];
    EXPECT_EQ(rm.materialId, 5);
    EXPECT_FLOAT_EQ(rm.roughness, 0.3f);
    EXPECT_FLOAT_EQ(rm.metallic, 0.8f);
    EXPECT_FLOAT_EQ(rm.specularStrength, 0.7f);
    EXPECT_FLOAT_EQ(rm.normalStrength, 0.9f);
    EXPECT_FLOAT_EQ(rm.detailScale, 2.0f);
    EXPECT_FLOAT_EQ(rm.emissionStrength, 0.5f);
    EXPECT_FLOAT_EQ(rm.aoStrength, 0.6f);

    ASSERT_EQ(rm.textureSlots.size(), 1u);
    const auto& rs = rm.textureSlots[0];
    EXPECT_EQ(rs.slotType, TextureSlotType::BASE_COLOR);
    EXPECT_EQ(rs.width, 2u);
    EXPECT_EQ(rs.height, 2u);
    EXPECT_EQ(rs.wrapS, 0x2901u);
    EXPECT_EQ(rs.wrapT, 0x812Fu);
    EXPECT_EQ(rs.minFilter, 0x2601u);
    EXPECT_EQ(rs.magFilter, 0x2600u);
    ASSERT_EQ(rs.pixelData.size(), 16u);
    EXPECT_EQ(rs.pixelData[0], 0xFF);
    EXPECT_EQ(rs.pixelData[4], 0x00);
}

TEST(SceneDataPacketTest, MultipleMaterialsRoundTrip)
{
    SceneDataPacket packet;

    MapbinMaterial mat1;
    mat1.materialId = 1;
    mat1.roughness = 0.2f;

    MapbinTextureSlot slot1;
    slot1.slotType = TextureSlotType::BASE_COLOR;
    slot1.width = 1;
    slot1.height = 1;
    slot1.pixelData = {0xAA, 0xBB, 0xCC, 0xDD};
    mat1.textureSlots.push_back(slot1);
    packet.materials.push_back(mat1);

    MapbinMaterial mat2;
    mat2.materialId = 2;
    mat2.roughness = 0.9f;

    MapbinTextureSlot slot2;
    slot2.slotType = TextureSlotType::NORMAL;
    slot2.width = 4;
    slot2.height = 4;
    slot2.wrapS = 0x812F;
    slot2.pixelData.resize(64, 0x42);
    mat2.textureSlots.push_back(slot2);
    packet.materials.push_back(mat2);

    auto result = RoundTrip(packet);
    ASSERT_EQ(result.materials.size(), 2u);
    EXPECT_EQ(result.materials[0].materialId, 1);
    EXPECT_FLOAT_EQ(result.materials[0].roughness, 0.2f);
    EXPECT_EQ(result.materials[0].textureSlots[0].pixelData.size(), 4u);
    EXPECT_EQ(result.materials[1].materialId, 2);
    EXPECT_FLOAT_EQ(result.materials[1].roughness, 0.9f);
    EXPECT_EQ(result.materials[1].textureSlots[0].slotType, TextureSlotType::NORMAL);
    EXPECT_EQ(result.materials[1].textureSlots[0].width, 4u);
    EXPECT_EQ(result.materials[1].textureSlots[0].wrapS, 0x812Fu);
    EXPECT_EQ(result.materials[1].textureSlots[0].pixelData.size(), 64u);
}

TEST(SceneDataPacketTest, MaterialWithMultipleSlots)
{
    SceneDataPacket packet;

    MapbinMaterial mat;
    mat.materialId = 10;

    MapbinTextureSlot baseSlot;
    baseSlot.slotType = TextureSlotType::BASE_COLOR;
    baseSlot.width = 1;
    baseSlot.height = 1;
    baseSlot.pixelData = {0xFF, 0xFF, 0xFF, 0xFF};
    mat.textureSlots.push_back(baseSlot);

    MapbinTextureSlot normalSlot;
    normalSlot.slotType = TextureSlotType::NORMAL;
    normalSlot.width = 2;
    normalSlot.height = 2;
    normalSlot.pixelData.resize(16, 0x80);
    mat.textureSlots.push_back(normalSlot);

    packet.materials.push_back(mat);

    auto result = RoundTrip(packet);
    ASSERT_EQ(result.materials.size(), 1u);
    ASSERT_EQ(result.materials[0].textureSlots.size(), 2u);
    EXPECT_EQ(result.materials[0].textureSlots[0].slotType, TextureSlotType::BASE_COLOR);
    EXPECT_EQ(result.materials[0].textureSlots[1].slotType, TextureSlotType::NORMAL);
    EXPECT_EQ(result.materials[0].textureSlots[1].width, 2u);
}

TEST(SceneDataPacketTest, SingleGroupRoundTrip)
{
    SceneDataPacket packet;

    MapbinGroup group;
    group.materialId = 7;
    group.vertexCount = 3;
    group.vertexData = {1.0f, 2.0f, 3.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
                        4.0f, 5.0f, 6.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
                        7.0f, 8.0f, 9.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};
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

TEST(SceneDataPacketTest, MaterialsAndGroupsCombined)
{
    SceneDataPacket packet;

    MapbinMaterial mat;
    mat.materialId = 10;
    MapbinTextureSlot slot;
    slot.slotType = TextureSlotType::BASE_COLOR;
    slot.width = 1;
    slot.height = 1;
    slot.pixelData = {0xFF, 0xFF, 0xFF, 0xFF};
    mat.textureSlots.push_back(slot);
    packet.materials.push_back(mat);

    MapbinGroup group;
    group.materialId = 10;
    group.vertexCount = 3;
    group.vertexData.resize(24, 0.5f);
    group.indices = {0, 2, 1};
    packet.groups.push_back(group);

    auto result = RoundTrip(packet);
    ASSERT_EQ(result.materials.size(), 1u);
    ASSERT_EQ(result.groups.size(), 1u);
    EXPECT_EQ(result.materials[0].materialId, 10);
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

    MapbinMaterial mat;
    mat.materialId = 0;

    MapbinTextureSlot slot;
    slot.slotType = TextureSlotType::BASE_COLOR;
    slot.width = 32;
    slot.height = 32;
    slot.pixelData.resize(32 * 32 * 4);

    for (size_t i = 0; i < slot.pixelData.size(); ++i)
        slot.pixelData[i] = static_cast<uint8_t>(i & 0xFF);

    mat.textureSlots.push_back(slot);
    packet.materials.push_back(mat);

    auto result = RoundTrip(packet);
    ASSERT_EQ(result.materials.size(), 1u);
    ASSERT_EQ(result.materials[0].textureSlots.size(), 1u);
    ASSERT_EQ(result.materials[0].textureSlots[0].pixelData.size(), 32u * 32u * 4u);

    for (size_t i = 0; i < result.materials[0].textureSlots[0].pixelData.size(); ++i)
        EXPECT_EQ(result.materials[0].textureSlots[0].pixelData[i], static_cast<uint8_t>(i & 0xFF));
}

TEST(SceneDataPacketTest, MaterialScalarFieldsPreserved)
{
    SceneDataPacket packet;

    MapbinMaterial mat;
    mat.materialId = 42;
    mat.normalStrength = 0.8f;
    mat.roughness = 0.3f;
    mat.metallic = 0.9f;
    mat.specularStrength = 0.7f;
    mat.detailScale = 3.0f;
    mat.emissionStrength = 0.4f;
    mat.aoStrength = 0.6f;
    packet.materials.push_back(mat);

    auto result = RoundTrip(packet);
    ASSERT_EQ(result.materials.size(), 1u);
    const auto& rm = result.materials[0];
    EXPECT_EQ(rm.materialId, 42);
    EXPECT_FLOAT_EQ(rm.normalStrength, 0.8f);
    EXPECT_FLOAT_EQ(rm.roughness, 0.3f);
    EXPECT_FLOAT_EQ(rm.metallic, 0.9f);
    EXPECT_FLOAT_EQ(rm.specularStrength, 0.7f);
    EXPECT_FLOAT_EQ(rm.detailScale, 3.0f);
    EXPECT_FLOAT_EQ(rm.emissionStrength, 0.4f);
    EXPECT_FLOAT_EQ(rm.aoStrength, 0.6f);
}

TEST(SceneDataPacketTest, EmptyTextureSlots)
{
    SceneDataPacket packet;

    MapbinMaterial mat;
    mat.materialId = 0;
    packet.materials.push_back(mat);

    auto result = RoundTrip(packet);
    ASSERT_EQ(result.materials.size(), 1u);
    EXPECT_TRUE(result.materials[0].textureSlots.empty());
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
            static_cast<float>(i * 10 + 2), 0.0f, 1.0f, 0.0f, 0.0f, 0.0f
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
