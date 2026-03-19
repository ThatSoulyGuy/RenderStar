#include <gtest/gtest.h>
#include "RenderStar/Common/Scene/MapbinLoader.hpp"
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

    void WriteV3Header(std::vector<uint8_t>& buf, uint32_t materialCount, uint32_t groupCount)
    {
        WriteUint32(buf, 0x4D415042);
        WriteUint32(buf, 3);
        WriteUint32(buf, materialCount);
        WriteUint32(buf, groupCount);
    }

    void WriteV3MaterialScalars(std::vector<uint8_t>& buf, int32_t materialId,
        float normalStr, float roughness, float metallic, float specularStr,
        float detailScale, float emissionStr, float aoStr, uint32_t slotCount)
    {
        WriteUint32(buf, static_cast<uint32_t>(materialId));
        WriteFloat(buf, normalStr);
        WriteFloat(buf, roughness);
        WriteFloat(buf, metallic);
        WriteFloat(buf, specularStr);
        WriteFloat(buf, detailScale);
        WriteFloat(buf, emissionStr);
        WriteFloat(buf, aoStr);
        WriteUint32(buf, slotCount);
    }

    void WriteV3TextureSlot(std::vector<uint8_t>& buf, uint32_t slotType,
        uint32_t width, uint32_t height, uint32_t wrapS, uint32_t wrapT,
        uint32_t minFilter, uint32_t magFilter, const std::vector<uint8_t>& pixels)
    {
        WriteUint32(buf, slotType);
        WriteUint32(buf, width);
        WriteUint32(buf, height);
        WriteUint32(buf, wrapS);
        WriteUint32(buf, wrapT);
        WriteUint32(buf, minFilter);
        WriteUint32(buf, magFilter);
        WriteUint32(buf, static_cast<uint32_t>(pixels.size()));
        buf.insert(buf.end(), pixels.begin(), pixels.end());
    }

    void WriteV3Vertex(std::vector<uint8_t>& buf, float px, float py, float pz,
                       float nx, float ny, float nz, float u, float v)
    {
        WriteFloat(buf, px);
        WriteFloat(buf, py);
        WriteFloat(buf, pz);
        WriteFloat(buf, nx);
        WriteFloat(buf, ny);
        WriteFloat(buf, nz);
        WriteFloat(buf, u);
        WriteFloat(buf, v);
    }
}

TEST(MapbinLoaderV3Test, EmptyScene)
{
    std::vector<uint8_t> buf;
    WriteV3Header(buf, 0, 0);

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->materials.empty());
    EXPECT_TRUE(result->groups.empty());
}

TEST(MapbinLoaderV3Test, MaterialScalarValues)
{
    std::vector<uint8_t> buf;
    WriteV3Header(buf, 1, 0);
    WriteV3MaterialScalars(buf, 42, 0.8f, 0.3f, 0.9f, 0.7f, 2.0f, 0.5f, 0.6f, 0);

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->materials.size(), 1u);

    const auto& mat = result->materials[0];
    EXPECT_EQ(mat.materialId, 42);
    EXPECT_FLOAT_EQ(mat.normalStrength, 0.8f);
    EXPECT_FLOAT_EQ(mat.roughness, 0.3f);
    EXPECT_FLOAT_EQ(mat.metallic, 0.9f);
    EXPECT_FLOAT_EQ(mat.specularStrength, 0.7f);
    EXPECT_FLOAT_EQ(mat.detailScale, 2.0f);
    EXPECT_FLOAT_EQ(mat.emissionStrength, 0.5f);
    EXPECT_FLOAT_EQ(mat.aoStrength, 0.6f);
    EXPECT_TRUE(mat.textureSlots.empty());
}

TEST(MapbinLoaderV3Test, SingleTextureSlot)
{
    std::vector<uint8_t> buf;
    WriteV3Header(buf, 1, 0);
    WriteV3MaterialScalars(buf, 1, 1.0f, 0.5f, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f, 1);

    std::vector<uint8_t> pixels = {0xFF, 0x00, 0xFF, 0xFF};
    WriteV3TextureSlot(buf, 0, 1, 1, 0x2901, 0x2901, 0x2601, 0x2601, pixels);

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->materials.size(), 1u);
    ASSERT_EQ(result->materials[0].textureSlots.size(), 1u);

    const auto& slot = result->materials[0].textureSlots[0];
    EXPECT_EQ(slot.slotType, TextureSlotType::BASE_COLOR);
    EXPECT_EQ(slot.width, 1u);
    EXPECT_EQ(slot.height, 1u);
    EXPECT_EQ(slot.wrapS, 0x2901u);
    EXPECT_EQ(slot.wrapT, 0x2901u);
    EXPECT_EQ(slot.minFilter, 0x2601u);
    EXPECT_EQ(slot.magFilter, 0x2601u);
    ASSERT_EQ(slot.pixelData.size(), 4u);
}

TEST(MapbinLoaderV3Test, MultipleTextureSlots)
{
    std::vector<uint8_t> buf;
    WriteV3Header(buf, 1, 0);
    WriteV3MaterialScalars(buf, 5, 1.0f, 0.5f, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f, 3);

    std::vector<uint8_t> pixels4(4, 0xAA);
    WriteV3TextureSlot(buf, 0, 1, 1, 0, 0, 0, 0, pixels4);
    WriteV3TextureSlot(buf, 1, 2, 2, 0, 0, 0, 0, std::vector<uint8_t>(16, 0xBB));
    WriteV3TextureSlot(buf, 2, 1, 1, 0, 0, 0, 0, std::vector<uint8_t>(4, 0xCC));

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->materials[0].textureSlots.size(), 3u);
    EXPECT_EQ(result->materials[0].textureSlots[0].slotType, TextureSlotType::BASE_COLOR);
    EXPECT_EQ(result->materials[0].textureSlots[1].slotType, TextureSlotType::NORMAL);
    EXPECT_EQ(result->materials[0].textureSlots[2].slotType, TextureSlotType::ROUGHNESS);
    EXPECT_EQ(result->materials[0].textureSlots[1].width, 2u);
    EXPECT_EQ(result->materials[0].textureSlots[1].height, 2u);
}

TEST(MapbinLoaderV3Test, AllSlotTypes)
{
    std::vector<uint8_t> buf;
    WriteV3Header(buf, 1, 0);
    WriteV3MaterialScalars(buf, 1, 1.0f, 0.5f, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f, 6);

    std::vector<uint8_t> px(4, 0x00);
    WriteV3TextureSlot(buf, 0, 1, 1, 0, 0, 0, 0, px);
    WriteV3TextureSlot(buf, 1, 1, 1, 0, 0, 0, 0, px);
    WriteV3TextureSlot(buf, 3, 1, 1, 0, 0, 0, 0, px);
    WriteV3TextureSlot(buf, 8, 1, 1, 0, 0, 0, 0, px);
    WriteV3TextureSlot(buf, 9, 1, 1, 0, 0, 0, 0, px);
    WriteV3TextureSlot(buf, 6, 1, 1, 0, 0, 0, 0, px);

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    auto& slots = result->materials[0].textureSlots;
    ASSERT_EQ(slots.size(), 6u);
    EXPECT_EQ(slots[0].slotType, TextureSlotType::BASE_COLOR);
    EXPECT_EQ(slots[1].slotType, TextureSlotType::NORMAL);
    EXPECT_EQ(slots[2].slotType, TextureSlotType::METALLIC);
    EXPECT_EQ(slots[3].slotType, TextureSlotType::EMISSION);
    EXPECT_EQ(slots[4].slotType, TextureSlotType::AO);
    EXPECT_EQ(slots[5].slotType, TextureSlotType::DETAIL_ALBEDO);
}

TEST(MapbinLoaderV3Test, MultipleMaterials)
{
    std::vector<uint8_t> buf;
    WriteV3Header(buf, 2, 0);
    WriteV3MaterialScalars(buf, 10, 1.0f, 0.2f, 0.8f, 0.5f, 1.0f, 0.0f, 1.0f, 0);
    WriteV3MaterialScalars(buf, 20, 0.5f, 0.9f, 0.1f, 0.3f, 3.0f, 1.0f, 0.5f, 0);

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->materials.size(), 2u);
    EXPECT_EQ(result->materials[0].materialId, 10);
    EXPECT_FLOAT_EQ(result->materials[0].roughness, 0.2f);
    EXPECT_FLOAT_EQ(result->materials[0].metallic, 0.8f);
    EXPECT_EQ(result->materials[1].materialId, 20);
    EXPECT_FLOAT_EQ(result->materials[1].roughness, 0.9f);
    EXPECT_FLOAT_EQ(result->materials[1].metallic, 0.1f);
}

TEST(MapbinLoaderV3Test, VertexDataWithNormals)
{
    std::vector<uint8_t> buf;
    WriteV3Header(buf, 0, 1);

    WriteUint32(buf, 0);
    WriteUint32(buf, 2);
    WriteUint32(buf, 0);

    WriteV3Vertex(buf, 1.0f, 2.0f, 3.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.75f);
    WriteV3Vertex(buf, 4.0f, 5.0f, 6.0f, 0.0f, 0.0f, 1.0f, 0.25f, 0.5f);

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->groups.size(), 1u);

    const auto& vd = result->groups[0].vertexData;
    ASSERT_EQ(vd.size(), 16u);

    EXPECT_FLOAT_EQ(vd[0], 1.0f);
    EXPECT_FLOAT_EQ(vd[1], 2.0f);
    EXPECT_FLOAT_EQ(vd[2], 3.0f);
    EXPECT_FLOAT_EQ(vd[3], 0.0f);
    EXPECT_FLOAT_EQ(vd[4], 1.0f);
    EXPECT_FLOAT_EQ(vd[5], 0.0f);
    EXPECT_FLOAT_EQ(vd[6], 0.5f);
    EXPECT_FLOAT_EQ(vd[7], 0.75f);

    EXPECT_FLOAT_EQ(vd[8], 4.0f);
    EXPECT_FLOAT_EQ(vd[9], 5.0f);
    EXPECT_FLOAT_EQ(vd[10], 6.0f);
    EXPECT_FLOAT_EQ(vd[11], 0.0f);
    EXPECT_FLOAT_EQ(vd[12], 0.0f);
    EXPECT_FLOAT_EQ(vd[13], 1.0f);
    EXPECT_FLOAT_EQ(vd[14], 0.25f);
    EXPECT_FLOAT_EQ(vd[15], 0.5f);
}

TEST(MapbinLoaderV3Test, NoWindingFlip)
{
    std::vector<uint8_t> buf;
    WriteV3Header(buf, 0, 1);

    WriteUint32(buf, 0);
    WriteUint32(buf, 3);
    WriteUint32(buf, 6);

    WriteV3Vertex(buf, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
    WriteV3Vertex(buf, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
    WriteV3Vertex(buf, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);

    WriteUint32(buf, 0);
    WriteUint32(buf, 1);
    WriteUint32(buf, 2);
    WriteUint32(buf, 2);
    WriteUint32(buf, 1);
    WriteUint32(buf, 0);

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->groups[0].indices.size(), 6u);
    EXPECT_EQ(result->groups[0].indices[0], 0u);
    EXPECT_EQ(result->groups[0].indices[1], 1u);
    EXPECT_EQ(result->groups[0].indices[2], 2u);
    EXPECT_EQ(result->groups[0].indices[3], 2u);
    EXPECT_EQ(result->groups[0].indices[4], 1u);
    EXPECT_EQ(result->groups[0].indices[5], 0u);
}

TEST(MapbinLoaderV3Test, MaterialWithGroupCombined)
{
    std::vector<uint8_t> buf;
    WriteV3Header(buf, 1, 1);
    WriteV3MaterialScalars(buf, 7, 1.0f, 0.5f, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f, 1);

    std::vector<uint8_t> pixels(4, 0xFF);
    WriteV3TextureSlot(buf, 0, 1, 1, 0x2901, 0x2901, 0x2601, 0x2601, pixels);

    WriteUint32(buf, 7);
    WriteUint32(buf, 3);
    WriteUint32(buf, 3);

    WriteV3Vertex(buf, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
    WriteV3Vertex(buf, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);
    WriteV3Vertex(buf, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);

    WriteUint32(buf, 0);
    WriteUint32(buf, 1);
    WriteUint32(buf, 2);

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->materials.size(), 1u);
    ASSERT_EQ(result->groups.size(), 1u);
    EXPECT_EQ(result->materials[0].materialId, 7);
    EXPECT_EQ(result->groups[0].materialId, 7);
    EXPECT_EQ(result->groups[0].vertexCount, 3);
}

TEST(MapbinLoaderV3Test, TruncatedMaterialReturnsNullopt)
{
    std::vector<uint8_t> buf;
    WriteV3Header(buf, 1, 0);
    buf.resize(buf.size() + 20, 0);

    auto result = MapbinLoader::Parse(buf);
    EXPECT_FALSE(result.has_value());
}

TEST(MapbinLoaderV3Test, TruncatedSlotReturnsNullopt)
{
    std::vector<uint8_t> buf;
    WriteV3Header(buf, 1, 0);
    WriteV3MaterialScalars(buf, 1, 1.0f, 0.5f, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f, 1);
    buf.resize(buf.size() + 16, 0);

    auto result = MapbinLoader::Parse(buf);
    EXPECT_FALSE(result.has_value());
}

TEST(MapbinLoaderV3Test, TruncatedSlotPixelsReturnsNullopt)
{
    std::vector<uint8_t> buf;
    WriteV3Header(buf, 1, 0);
    WriteV3MaterialScalars(buf, 1, 1.0f, 0.5f, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f, 1);

    WriteUint32(buf, 0);
    WriteUint32(buf, 2);
    WriteUint32(buf, 2);
    WriteUint32(buf, 0);
    WriteUint32(buf, 0);
    WriteUint32(buf, 0);
    WriteUint32(buf, 0);
    WriteUint32(buf, 9999);

    auto result = MapbinLoader::Parse(buf);
    EXPECT_FALSE(result.has_value());
}

TEST(MapbinLoaderV3Test, TruncatedGroupVerticesReturnsNullopt)
{
    std::vector<uint8_t> buf;
    WriteV3Header(buf, 0, 1);
    WriteUint32(buf, 0);
    WriteUint32(buf, 100);
    WriteUint32(buf, 3);

    auto result = MapbinLoader::Parse(buf);
    EXPECT_FALSE(result.has_value());
}

TEST(MapbinLoaderV3Test, GroupVertexCount)
{
    std::vector<uint8_t> buf;
    WriteV3Header(buf, 0, 1);

    WriteUint32(buf, 0);
    WriteUint32(buf, 4);
    WriteUint32(buf, 6);

    for (int i = 0; i < 4; ++i)
        WriteV3Vertex(buf, static_cast<float>(i), 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);

    for (uint32_t idx : {0u, 1u, 2u, 0u, 2u, 3u})
        WriteUint32(buf, idx);

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->groups[0].vertexCount, 4);
    EXPECT_EQ(result->groups[0].vertexData.size(), 32u);
    EXPECT_EQ(result->groups[0].indices.size(), 6u);
}
