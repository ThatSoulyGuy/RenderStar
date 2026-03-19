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

    void WriteHeader(std::vector<uint8_t>& buf, uint32_t version, uint32_t materialCount, uint32_t groupCount)
    {
        WriteUint32(buf, 0x4D415042);
        WriteUint32(buf, version);
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
        uint32_t width, uint32_t height, const std::vector<uint8_t>& pixels)
    {
        WriteUint32(buf, slotType);
        WriteUint32(buf, width);
        WriteUint32(buf, height);
        WriteUint32(buf, 0x2901);
        WriteUint32(buf, 0x2901);
        WriteUint32(buf, 0x2601);
        WriteUint32(buf, 0x2601);
        WriteUint32(buf, static_cast<uint32_t>(pixels.size()));
        buf.insert(buf.end(), pixels.begin(), pixels.end());
    }

    void WriteV4GameObject(std::vector<uint8_t>& buf, uint32_t type, float x, float y, float z)
    {
        WriteUint32(buf, type);
        WriteFloat(buf, x);
        WriteFloat(buf, y);
        WriteFloat(buf, z);
    }

    void WriteV5GameObjectCommon(std::vector<uint8_t>& buf, uint32_t type,
        float x, float y, float z, float rx, float ry, float rz)
    {
        WriteUint32(buf, type);
        WriteFloat(buf, x);
        WriteFloat(buf, y);
        WriteFloat(buf, z);
        WriteFloat(buf, rx);
        WriteFloat(buf, ry);
        WriteFloat(buf, rz);
    }

    void WriteV5LightFields(std::vector<uint8_t>& buf, float r, float g, float b, float intensity)
    {
        WriteFloat(buf, r);
        WriteFloat(buf, g);
        WriteFloat(buf, b);
        WriteFloat(buf, intensity);
    }

    void WriteV5SpotFields(std::vector<uint8_t>& buf, float innerCone, float outerCone)
    {
        WriteFloat(buf, innerCone);
        WriteFloat(buf, outerCone);
    }
}

TEST(MapbinLoaderV4Test, EmptyWithGameObjects)
{
    std::vector<uint8_t> buf;
    WriteHeader(buf, 4, 0, 0);
    WriteUint32(buf, 0);

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->materials.empty());
    EXPECT_TRUE(result->groups.empty());
    EXPECT_TRUE(result->gameObjects.empty());
}

TEST(MapbinLoaderV4Test, PlayerStartGameObject)
{
    std::vector<uint8_t> buf;
    WriteHeader(buf, 4, 0, 0);
    WriteUint32(buf, 1);
    WriteV4GameObject(buf, 0, 5.0f, 10.0f, 15.0f);

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->gameObjects.size(), 1u);

    auto& obj = result->gameObjects[0];
    EXPECT_EQ(obj.type, GameObjectType::PLAYER_START);
    EXPECT_FLOAT_EQ(obj.posX, 5.0f);
    EXPECT_FLOAT_EQ(obj.posY, 10.0f);
    EXPECT_FLOAT_EQ(obj.posZ, 15.0f);
    EXPECT_FLOAT_EQ(obj.rotX, 0.0f);
    EXPECT_FLOAT_EQ(obj.rotY, 0.0f);
    EXPECT_FLOAT_EQ(obj.rotZ, 0.0f);
}

TEST(MapbinLoaderV4Test, PointLightPositionOnly)
{
    std::vector<uint8_t> buf;
    WriteHeader(buf, 4, 0, 0);
    WriteUint32(buf, 1);
    WriteV4GameObject(buf, 1, 3.0f, 7.0f, 11.0f);

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->gameObjects.size(), 1u);

    auto& obj = result->gameObjects[0];
    EXPECT_EQ(obj.type, GameObjectType::POINT_LIGHT);
    EXPECT_FLOAT_EQ(obj.posX, 3.0f);
    EXPECT_FLOAT_EQ(obj.colorR, 1.0f);
    EXPECT_FLOAT_EQ(obj.intensity, 1.0f);
}

TEST(MapbinLoaderV4Test, MultipleGameObjects)
{
    std::vector<uint8_t> buf;
    WriteHeader(buf, 4, 0, 0);
    WriteUint32(buf, 3);
    WriteV4GameObject(buf, 0, 1.0f, 2.0f, 3.0f);
    WriteV4GameObject(buf, 1, 4.0f, 5.0f, 6.0f);
    WriteV4GameObject(buf, 3, 7.0f, 8.0f, 9.0f);

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->gameObjects.size(), 3u);
    EXPECT_EQ(result->gameObjects[0].type, GameObjectType::PLAYER_START);
    EXPECT_EQ(result->gameObjects[1].type, GameObjectType::POINT_LIGHT);
    EXPECT_EQ(result->gameObjects[2].type, GameObjectType::SUN_LIGHT);
}

TEST(MapbinLoaderV4Test, TruncatedGameObjectTable)
{
    std::vector<uint8_t> buf;
    WriteHeader(buf, 4, 0, 0);
    WriteUint32(buf, 2);
    WriteV4GameObject(buf, 0, 1.0f, 2.0f, 3.0f);

    auto result = MapbinLoader::Parse(buf);
    EXPECT_FALSE(result.has_value());
}

TEST(MapbinLoaderV4Test, MaterialsAndGroupsStillParse)
{
    std::vector<uint8_t> buf;
    WriteHeader(buf, 4, 1, 0);

    std::vector<uint8_t> pixels = { 255, 0, 0, 255 };
    WriteV3MaterialScalars(buf, 0, 1.0f, 0.5f, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f, 1);
    WriteV3TextureSlot(buf, 0, 1, 1, pixels);

    WriteUint32(buf, 0);

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->materials.size(), 1u);
    EXPECT_EQ(result->materials[0].materialId, 0);
    EXPECT_EQ(result->materials[0].textureSlots.size(), 1u);
    EXPECT_TRUE(result->gameObjects.empty());
}

TEST(MapbinLoaderV5Test, EmptyScene)
{
    std::vector<uint8_t> buf;
    WriteHeader(buf, 5, 0, 0);
    WriteUint32(buf, 0);

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->gameObjects.empty());
}

TEST(MapbinLoaderV5Test, PlayerStartWithRotation)
{
    std::vector<uint8_t> buf;
    WriteHeader(buf, 5, 0, 0);
    WriteUint32(buf, 1);
    WriteV5GameObjectCommon(buf, 0, 5.0f, 0.0f, 10.0f, 0.0f, 90.0f, 0.0f);

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->gameObjects.size(), 1u);

    auto& obj = result->gameObjects[0];
    EXPECT_EQ(obj.type, GameObjectType::PLAYER_START);
    EXPECT_FLOAT_EQ(obj.posX, 5.0f);
    EXPECT_FLOAT_EQ(obj.posZ, 10.0f);
    EXPECT_FLOAT_EQ(obj.rotY, 90.0f);
}

TEST(MapbinLoaderV5Test, PointLight)
{
    std::vector<uint8_t> buf;
    WriteHeader(buf, 5, 0, 0);
    WriteUint32(buf, 1);
    WriteV5GameObjectCommon(buf, 1, 3.0f, 5.0f, 7.0f, 0.0f, 0.0f, 0.0f);
    WriteV5LightFields(buf, 1.0f, 0.8f, 0.6f, 2.5f);

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->gameObjects.size(), 1u);

    auto& obj = result->gameObjects[0];
    EXPECT_EQ(obj.type, GameObjectType::POINT_LIGHT);
    EXPECT_FLOAT_EQ(obj.posX, 3.0f);
    EXPECT_FLOAT_EQ(obj.posY, 5.0f);
    EXPECT_FLOAT_EQ(obj.posZ, 7.0f);
    EXPECT_FLOAT_EQ(obj.colorR, 1.0f);
    EXPECT_FLOAT_EQ(obj.colorG, 0.8f);
    EXPECT_FLOAT_EQ(obj.colorB, 0.6f);
    EXPECT_FLOAT_EQ(obj.intensity, 2.5f);
}

TEST(MapbinLoaderV5Test, SpotLightWithCone)
{
    std::vector<uint8_t> buf;
    WriteHeader(buf, 5, 0, 0);
    WriteUint32(buf, 1);
    WriteV5GameObjectCommon(buf, 2, 1.0f, 8.0f, 2.0f, 45.0f, 30.0f, 0.0f);
    WriteV5LightFields(buf, 1.0f, 1.0f, 0.9f, 3.0f);
    WriteV5SpotFields(buf, 20.0f, 35.0f);

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->gameObjects.size(), 1u);

    auto& obj = result->gameObjects[0];
    EXPECT_EQ(obj.type, GameObjectType::SPOT_LIGHT);
    EXPECT_FLOAT_EQ(obj.posX, 1.0f);
    EXPECT_FLOAT_EQ(obj.posY, 8.0f);
    EXPECT_FLOAT_EQ(obj.rotX, 45.0f);
    EXPECT_FLOAT_EQ(obj.rotY, 30.0f);
    EXPECT_FLOAT_EQ(obj.colorR, 1.0f);
    EXPECT_FLOAT_EQ(obj.colorB, 0.9f);
    EXPECT_FLOAT_EQ(obj.intensity, 3.0f);
    EXPECT_FLOAT_EQ(obj.innerCone, 20.0f);
    EXPECT_FLOAT_EQ(obj.outerCone, 35.0f);
}

TEST(MapbinLoaderV5Test, SunLightWithRotation)
{
    std::vector<uint8_t> buf;
    WriteHeader(buf, 5, 0, 0);
    WriteUint32(buf, 1);
    WriteV5GameObjectCommon(buf, 3, 0.0f, 100.0f, 0.0f, -45.0f, 60.0f, 0.0f);
    WriteV5LightFields(buf, 1.0f, 0.95f, 0.8f, 1.2f);

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->gameObjects.size(), 1u);

    auto& obj = result->gameObjects[0];
    EXPECT_EQ(obj.type, GameObjectType::SUN_LIGHT);
    EXPECT_FLOAT_EQ(obj.rotX, -45.0f);
    EXPECT_FLOAT_EQ(obj.rotY, 60.0f);
    EXPECT_FLOAT_EQ(obj.colorG, 0.95f);
    EXPECT_FLOAT_EQ(obj.intensity, 1.2f);
}

TEST(MapbinLoaderV5Test, AllGameObjectTypes)
{
    std::vector<uint8_t> buf;
    WriteHeader(buf, 5, 0, 0);
    WriteUint32(buf, 4);

    WriteV5GameObjectCommon(buf, 0, 0.0f, 0.0f, 0.0f, 0.0f, 90.0f, 0.0f);

    WriteV5GameObjectCommon(buf, 1, 5.0f, 3.0f, 5.0f, 0.0f, 0.0f, 0.0f);
    WriteV5LightFields(buf, 1.0f, 0.5f, 0.0f, 2.0f);

    WriteV5GameObjectCommon(buf, 2, 10.0f, 6.0f, 10.0f, 30.0f, 45.0f, 0.0f);
    WriteV5LightFields(buf, 1.0f, 1.0f, 1.0f, 1.5f);
    WriteV5SpotFields(buf, 15.0f, 30.0f);

    WriteV5GameObjectCommon(buf, 3, 0.0f, 50.0f, 0.0f, -60.0f, 0.0f, 0.0f);
    WriteV5LightFields(buf, 1.0f, 1.0f, 0.9f, 1.0f);

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->gameObjects.size(), 4u);

    EXPECT_EQ(result->gameObjects[0].type, GameObjectType::PLAYER_START);
    EXPECT_EQ(result->gameObjects[1].type, GameObjectType::POINT_LIGHT);
    EXPECT_EQ(result->gameObjects[2].type, GameObjectType::SPOT_LIGHT);
    EXPECT_EQ(result->gameObjects[3].type, GameObjectType::SUN_LIGHT);

    EXPECT_FLOAT_EQ(result->gameObjects[2].innerCone, 15.0f);
    EXPECT_FLOAT_EQ(result->gameObjects[2].outerCone, 30.0f);
}

TEST(MapbinLoaderV5Test, TruncatedCommonFields)
{
    std::vector<uint8_t> buf;
    WriteHeader(buf, 5, 0, 0);
    WriteUint32(buf, 1);
    WriteUint32(buf, 0);
    WriteFloat(buf, 1.0f);

    auto result = MapbinLoader::Parse(buf);
    EXPECT_FALSE(result.has_value());
}

TEST(MapbinLoaderV5Test, TruncatedLightFields)
{
    std::vector<uint8_t> buf;
    WriteHeader(buf, 5, 0, 0);
    WriteUint32(buf, 1);
    WriteV5GameObjectCommon(buf, 1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    WriteFloat(buf, 1.0f);
    WriteFloat(buf, 1.0f);

    auto result = MapbinLoader::Parse(buf);
    EXPECT_FALSE(result.has_value());
}

TEST(MapbinLoaderV5Test, TruncatedSpotFields)
{
    std::vector<uint8_t> buf;
    WriteHeader(buf, 5, 0, 0);
    WriteUint32(buf, 1);
    WriteV5GameObjectCommon(buf, 2, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    WriteV5LightFields(buf, 1.0f, 1.0f, 1.0f, 1.0f);
    WriteFloat(buf, 20.0f);

    auto result = MapbinLoader::Parse(buf);
    EXPECT_FALSE(result.has_value());
}

TEST(MapbinLoaderV5Test, MaterialsAndGroupsStillParse)
{
    std::vector<uint8_t> buf;
    WriteHeader(buf, 5, 1, 0);

    std::vector<uint8_t> pixels = { 255, 128, 64, 255 };
    WriteV3MaterialScalars(buf, 42, 0.8f, 0.3f, 0.1f, 0.7f, 2.0f, 0.5f, 0.9f, 1);
    WriteV3TextureSlot(buf, 0, 1, 1, pixels);

    WriteUint32(buf, 1);
    WriteV5GameObjectCommon(buf, 1, 1.0f, 2.0f, 3.0f, 0.0f, 0.0f, 0.0f);
    WriteV5LightFields(buf, 1.0f, 1.0f, 1.0f, 1.0f);

    auto result = MapbinLoader::Parse(buf);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->materials.size(), 1u);
    EXPECT_EQ(result->materials[0].materialId, 42);
    EXPECT_FLOAT_EQ(result->materials[0].roughness, 0.3f);
    EXPECT_FLOAT_EQ(result->materials[0].detailScale, 2.0f);
    ASSERT_EQ(result->gameObjects.size(), 1u);
    EXPECT_EQ(result->gameObjects[0].type, GameObjectType::POINT_LIGHT);
}

TEST(MapbinLoaderV5Test, GameObjectTypeEnumValues)
{
    EXPECT_EQ(static_cast<uint32_t>(GameObjectType::PLAYER_START), 0u);
    EXPECT_EQ(static_cast<uint32_t>(GameObjectType::POINT_LIGHT), 1u);
    EXPECT_EQ(static_cast<uint32_t>(GameObjectType::SPOT_LIGHT), 2u);
    EXPECT_EQ(static_cast<uint32_t>(GameObjectType::SUN_LIGHT), 3u);
}

TEST(MapbinLoaderV5Test, GameObjectDefaultValues)
{
    MapbinGameObject obj;
    EXPECT_EQ(obj.type, GameObjectType::PLAYER_START);
    EXPECT_FLOAT_EQ(obj.posX, 0.0f);
    EXPECT_FLOAT_EQ(obj.posY, 0.0f);
    EXPECT_FLOAT_EQ(obj.posZ, 0.0f);
    EXPECT_FLOAT_EQ(obj.rotX, 0.0f);
    EXPECT_FLOAT_EQ(obj.rotY, 0.0f);
    EXPECT_FLOAT_EQ(obj.rotZ, 0.0f);
    EXPECT_FLOAT_EQ(obj.colorR, 1.0f);
    EXPECT_FLOAT_EQ(obj.colorG, 1.0f);
    EXPECT_FLOAT_EQ(obj.colorB, 1.0f);
    EXPECT_FLOAT_EQ(obj.intensity, 1.0f);
    EXPECT_FLOAT_EQ(obj.innerCone, 0.0f);
    EXPECT_FLOAT_EQ(obj.outerCone, 0.0f);
}
