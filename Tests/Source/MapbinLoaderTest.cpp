#include <gtest/gtest.h>
#include "RenderStar/Common/Scene/MapbinLoader.hpp"
#include <cmath>
#include <cstring>
#include <vector>

using namespace RenderStar::Common::Scene;

namespace
{
    void WriteUint32(std::vector<uint8_t>& buf, uint32_t value)
    {
        size_t offset = buf.size();
        buf.resize(buf.size() + 4);
        std::memcpy(buf.data() + offset, &value, sizeof(uint32_t));
    }

    void WriteFloat(std::vector<uint8_t>& buf, float value)
    {
        size_t offset = buf.size();
        buf.resize(buf.size() + 4);
        std::memcpy(buf.data() + offset, &value, sizeof(float));
    }

    void WriteV2Header(std::vector<uint8_t>& buf, uint32_t textureCount, uint32_t groupCount)
    {
        WriteUint32(buf, 0x4D415042);
        WriteUint32(buf, 2);
        WriteUint32(buf, textureCount);
        WriteUint32(buf, groupCount);
    }

    void WriteV2Texture(std::vector<uint8_t>& buf, int32_t materialId, uint32_t width, uint32_t height,
                        uint32_t wrapS, uint32_t wrapT, uint32_t minFilter, uint32_t magFilter,
                        const std::vector<uint8_t>& pixels)
    {
        WriteUint32(buf, static_cast<uint32_t>(materialId));
        WriteUint32(buf, width);
        WriteUint32(buf, height);
        WriteUint32(buf, wrapS);
        WriteUint32(buf, wrapT);
        WriteUint32(buf, minFilter);
        WriteUint32(buf, magFilter);
        WriteUint32(buf, static_cast<uint32_t>(pixels.size()));
        buf.insert(buf.end(), pixels.begin(), pixels.end());
    }

    void WriteV2Vertex(std::vector<uint8_t>& buf, float px, float py, float pz,
                       float cx, float cy, float cz, float u, float v)
    {
        WriteFloat(buf, px);
        WriteFloat(buf, py);
        WriteFloat(buf, pz);
        WriteFloat(buf, cx);
        WriteFloat(buf, cy);
        WriteFloat(buf, cz);
        WriteFloat(buf, u);
        WriteFloat(buf, v);
    }

    void WriteV2Group(std::vector<uint8_t>& buf, int32_t materialId, uint32_t vertexCount,
                      const std::vector<uint32_t>& indices)
    {
        WriteUint32(buf, static_cast<uint32_t>(materialId));
        WriteUint32(buf, vertexCount);
        WriteUint32(buf, static_cast<uint32_t>(indices.size()));

        for (uint32_t v = 0; v < vertexCount; ++v)
            WriteV2Vertex(buf, static_cast<float>(v), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

        for (uint32_t idx : indices)
            WriteUint32(buf, idx);
    }
}

TEST(MapbinLoaderTest, EmptyDataReturnsNullopt)
{
    auto result = MapbinLoader::Parse({});
    EXPECT_FALSE(result.has_value());
}

TEST(MapbinLoaderTest, TooSmallDataReturnsNullopt)
{
    std::vector<uint8_t> data(8, 0);
    auto result = MapbinLoader::Parse(data);
    EXPECT_FALSE(result.has_value());
}

TEST(MapbinLoaderTest, WrongMagicReturnsNullopt)
{
    std::vector<uint8_t> buf;
    WriteUint32(buf, 0xDEADBEEF);
    WriteUint32(buf, 2);
    WriteUint32(buf, 0);
    WriteUint32(buf, 0);

    auto result = MapbinLoader::Parse(buf);
    EXPECT_FALSE(result.has_value());
}

TEST(MapbinLoaderTest, WrongVersionReturnsNullopt)
{
    std::vector<uint8_t> buf;
    WriteUint32(buf, 0x4D415042);
    WriteUint32(buf, 99);
    WriteUint32(buf, 0);
    WriteUint32(buf, 0);

    auto result = MapbinLoader::Parse(buf);
    EXPECT_FALSE(result.has_value());
}

TEST(MapbinLoaderTest, EmptyScene)
{
    std::vector<uint8_t> buf;
    WriteV2Header(buf, 0, 0);

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->materials.empty());
    EXPECT_TRUE(result->groups.empty());
}

TEST(MapbinLoaderTest, V2TextureConvertedToMaterial)
{
    std::vector<uint8_t> buf;
    WriteV2Header(buf, 1, 0);

    std::vector<uint8_t> pixels = {0xFF, 0x00, 0xFF, 0xFF};
    WriteV2Texture(buf, 5, 1, 1, 0x2901, 0x2901, 0x2601, 0x2601, pixels);

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->materials.size(), 1u);

    const auto& mat = result->materials[0];
    EXPECT_EQ(mat.materialId, 5);
    EXPECT_FLOAT_EQ(mat.roughness, 0.5f);
    EXPECT_FLOAT_EQ(mat.metallic, 0.0f);
    EXPECT_FLOAT_EQ(mat.specularStrength, 0.5f);

    ASSERT_EQ(mat.textureSlots.size(), 1u);
    const auto& slot = mat.textureSlots[0];
    EXPECT_EQ(slot.slotType, TextureSlotType::BASE_COLOR);
    EXPECT_EQ(slot.width, 1u);
    EXPECT_EQ(slot.height, 1u);
    EXPECT_EQ(slot.wrapS, 0x2901u);
    EXPECT_EQ(slot.wrapT, 0x2901u);
    EXPECT_EQ(slot.minFilter, 0x2601u);
    EXPECT_EQ(slot.magFilter, 0x2601u);
    ASSERT_EQ(slot.pixelData.size(), 4u);
    EXPECT_EQ(slot.pixelData[0], 0xFF);
    EXPECT_EQ(slot.pixelData[1], 0x00);
    EXPECT_EQ(slot.pixelData[2], 0xFF);
    EXPECT_EQ(slot.pixelData[3], 0xFF);
}

TEST(MapbinLoaderTest, V2MultipleTexturesConvertedToMaterials)
{
    std::vector<uint8_t> buf;
    WriteV2Header(buf, 2, 0);

    std::vector<uint8_t> pixels1(16, 0xAA);
    WriteV2Texture(buf, 1, 2, 2, 0x2901, 0x812F, 0x2600, 0x2601, pixels1);

    std::vector<uint8_t> pixels2(4, 0xBB);
    WriteV2Texture(buf, 2, 1, 1, 0x812F, 0x812F, 0x2601, 0x2600, pixels2);

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->materials.size(), 2u);

    EXPECT_EQ(result->materials[0].materialId, 1);
    ASSERT_EQ(result->materials[0].textureSlots.size(), 1u);
    EXPECT_EQ(result->materials[0].textureSlots[0].width, 2u);
    EXPECT_EQ(result->materials[0].textureSlots[0].height, 2u);
    EXPECT_EQ(result->materials[0].textureSlots[0].wrapS, 0x2901u);
    EXPECT_EQ(result->materials[0].textureSlots[0].wrapT, 0x812Fu);
    EXPECT_EQ(result->materials[0].textureSlots[0].pixelData.size(), 16u);

    EXPECT_EQ(result->materials[1].materialId, 2);
    ASSERT_EQ(result->materials[1].textureSlots.size(), 1u);
    EXPECT_EQ(result->materials[1].textureSlots[0].width, 1u);
    EXPECT_EQ(result->materials[1].textureSlots[0].wrapS, 0x812Fu);
    EXPECT_EQ(result->materials[1].textureSlots[0].pixelData.size(), 4u);
}

TEST(MapbinLoaderTest, SingleGroupNoTextures)
{
    std::vector<uint8_t> buf;
    WriteV2Header(buf, 0, 1);
    WriteV2Group(buf, 3, 3, {0, 1, 2});

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->materials.empty());
    ASSERT_EQ(result->groups.size(), 1u);

    const auto& group = result->groups[0];
    EXPECT_EQ(group.materialId, 3);
    EXPECT_EQ(group.vertexCount, 3);
    ASSERT_EQ(group.indices.size(), 3u);
}

TEST(MapbinLoaderTest, GroupMaterialIdPreserved)
{
    std::vector<uint8_t> buf;
    WriteV2Header(buf, 0, 2);
    WriteV2Group(buf, 7, 3, {0, 1, 2});
    WriteV2Group(buf, 42, 3, {0, 1, 2});

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->groups.size(), 2u);
    EXPECT_EQ(result->groups[0].materialId, 7);
    EXPECT_EQ(result->groups[1].materialId, 42);
}

TEST(MapbinLoaderTest, V2WindingOrderSwapped)
{
    std::vector<uint8_t> buf;
    WriteV2Header(buf, 0, 1);
    WriteV2Group(buf, 0, 3, {0, 1, 2});

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->groups[0].indices.size(), 3u);
    EXPECT_EQ(result->groups[0].indices[0], 0u);
    EXPECT_EQ(result->groups[0].indices[1], 2u);
    EXPECT_EQ(result->groups[0].indices[2], 1u);
}

TEST(MapbinLoaderTest, V2WindingOrderSwappedMultipleTriangles)
{
    std::vector<uint8_t> buf;
    WriteV2Header(buf, 0, 1);
    WriteV2Group(buf, 0, 6, {0, 1, 2, 3, 4, 5});

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->groups[0].indices.size(), 6u);
    EXPECT_EQ(result->groups[0].indices[0], 0u);
    EXPECT_EQ(result->groups[0].indices[1], 2u);
    EXPECT_EQ(result->groups[0].indices[2], 1u);
    EXPECT_EQ(result->groups[0].indices[3], 3u);
    EXPECT_EQ(result->groups[0].indices[4], 5u);
    EXPECT_EQ(result->groups[0].indices[5], 4u);
}

TEST(MapbinLoaderTest, V2VertexPositionAndUV)
{
    std::vector<uint8_t> buf;
    WriteV2Header(buf, 0, 1);

    WriteUint32(buf, 0);
    WriteUint32(buf, 1);
    WriteUint32(buf, 0);

    WriteV2Vertex(buf, 1.0f, 2.0f, 3.0f, 0.6f, 0.8f, -0.2f, 0.5f, 0.75f);

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->groups.size(), 1u);

    const auto& vd = result->groups[0].vertexData;
    ASSERT_EQ(vd.size(), 8u);
    EXPECT_FLOAT_EQ(vd[0], 1.0f);
    EXPECT_FLOAT_EQ(vd[1], 2.0f);
    EXPECT_FLOAT_EQ(vd[2], 3.0f);
    EXPECT_FLOAT_EQ(vd[3], 0.0f);
    EXPECT_FLOAT_EQ(vd[4], 1.0f);
    EXPECT_FLOAT_EQ(vd[5], 0.0f);
    EXPECT_FLOAT_EQ(vd[6], 0.5f);
    EXPECT_FLOAT_EQ(vd[7], 0.75f);
}

TEST(MapbinLoaderTest, V2ComputedNormals)
{
    std::vector<uint8_t> buf;
    WriteV2Header(buf, 0, 1);

    WriteUint32(buf, 0);
    WriteUint32(buf, 3);
    WriteUint32(buf, 3);

    WriteV2Vertex(buf, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    WriteV2Vertex(buf, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    WriteV2Vertex(buf, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

    WriteUint32(buf, 0);
    WriteUint32(buf, 2);
    WriteUint32(buf, 1);

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->groups.size(), 1u);

    const auto& vd = result->groups[0].vertexData;

    float nx0 = vd[3], ny0 = vd[4], nz0 = vd[5];
    float nx1 = vd[11], ny1 = vd[12], nz1 = vd[13];
    float nx2 = vd[19], ny2 = vd[20], nz2 = vd[21];

    EXPECT_NEAR(ny0, -1.0f, 1e-5f);
    EXPECT_NEAR(ny1, -1.0f, 1e-5f);
    EXPECT_NEAR(ny2, -1.0f, 1e-5f);

    EXPECT_NEAR(nx0, 0.0f, 1e-5f);
    EXPECT_NEAR(nz0, 0.0f, 1e-5f);
}

TEST(MapbinLoaderTest, V2TextureAndGroupCombined)
{
    std::vector<uint8_t> buf;
    WriteV2Header(buf, 1, 1);

    std::vector<uint8_t> pixels(4, 0xFF);
    WriteV2Texture(buf, 10, 1, 1, 0x2901, 0x2901, 0x2601, 0x2601, pixels);
    WriteV2Group(buf, 10, 3, {0, 1, 2});

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->materials.size(), 1u);
    ASSERT_EQ(result->groups.size(), 1u);
    EXPECT_EQ(result->materials[0].materialId, 10);
    EXPECT_EQ(result->groups[0].materialId, 10);
}

TEST(MapbinLoaderTest, TruncatedTextureHeaderReturnsNullopt)
{
    std::vector<uint8_t> buf;
    WriteV2Header(buf, 1, 0);
    buf.resize(buf.size() + 16, 0);

    auto result = MapbinLoader::Parse(buf);
    EXPECT_FALSE(result.has_value());
}

TEST(MapbinLoaderTest, TruncatedTexturePixelsReturnsNullopt)
{
    std::vector<uint8_t> buf;
    WriteV2Header(buf, 1, 0);

    WriteUint32(buf, 0);
    WriteUint32(buf, 2);
    WriteUint32(buf, 2);
    WriteUint32(buf, 0);
    WriteUint32(buf, 0);
    WriteUint32(buf, 0);
    WriteUint32(buf, 0);
    WriteUint32(buf, 1000);

    auto result = MapbinLoader::Parse(buf);
    EXPECT_FALSE(result.has_value());
}

TEST(MapbinLoaderTest, TruncatedGroupHeaderReturnsNullopt)
{
    std::vector<uint8_t> buf;
    WriteV2Header(buf, 0, 1);
    buf.resize(buf.size() + 4, 0);

    auto result = MapbinLoader::Parse(buf);
    EXPECT_FALSE(result.has_value());
}

TEST(MapbinLoaderTest, TruncatedGroupVerticesReturnsNullopt)
{
    std::vector<uint8_t> buf;
    WriteV2Header(buf, 0, 1);
    WriteUint32(buf, 0);
    WriteUint32(buf, 100);
    WriteUint32(buf, 3);

    auto result = MapbinLoader::Parse(buf);
    EXPECT_FALSE(result.has_value());
}

TEST(MapbinLoaderTest, V2LargeTexturePixelData)
{
    std::vector<uint8_t> buf;
    WriteV2Header(buf, 1, 0);

    uint32_t width = 64;
    uint32_t height = 64;
    std::vector<uint8_t> pixels(width * height * 4, 0x80);
    WriteV2Texture(buf, 0, width, height, 0x2901, 0x2901, 0x2601, 0x2601, pixels);

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->materials.size(), 1u);
    ASSERT_EQ(result->materials[0].textureSlots.size(), 1u);
    EXPECT_EQ(result->materials[0].textureSlots[0].width, 64u);
    EXPECT_EQ(result->materials[0].textureSlots[0].height, 64u);
    EXPECT_EQ(result->materials[0].textureSlots[0].pixelData.size(), 64u * 64u * 4u);
}

TEST(MapbinLoaderTest, V2ZeroSizeTexturePixelData)
{
    std::vector<uint8_t> buf;
    WriteV2Header(buf, 1, 0);

    std::vector<uint8_t> pixels;
    WriteV2Texture(buf, 0, 0, 0, 0, 0, 0, 0, pixels);

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->materials.size(), 1u);
    ASSERT_EQ(result->materials[0].textureSlots.size(), 1u);
    EXPECT_TRUE(result->materials[0].textureSlots[0].pixelData.empty());
}
