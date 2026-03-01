#include <gtest/gtest.h>
#include "RenderStar/Client/Render/Framework/SceneLightingData.hpp"

using namespace RenderStar::Client::Render::Framework;

TEST(SceneLightingDataTest, DefaultValues)
{
    SceneLightingData data{};

    EXPECT_FLOAT_EQ(data.cameraPosition.x, 0.0f);
    EXPECT_FLOAT_EQ(data.cameraPosition.y, 0.0f);
    EXPECT_FLOAT_EQ(data.cameraPosition.z, 0.0f);
    EXPECT_FLOAT_EQ(data.cameraPosition.w, 0.0f);

    EXPECT_FLOAT_EQ(data.ambientColor.x, 0.0f);
    EXPECT_FLOAT_EQ(data.ambientColor.y, 0.0f);
    EXPECT_FLOAT_EQ(data.ambientColor.z, 0.0f);
    EXPECT_FLOAT_EQ(data.ambientColor.w, 0.0f);

    EXPECT_FLOAT_EQ(data.directionalDirection.x, 0.0f);
    EXPECT_FLOAT_EQ(data.directionalColor.x, 0.0f);

    EXPECT_EQ(data.pointLightCount, 0);
    EXPECT_EQ(data.spotLightCount, 0);
}

TEST(SceneLightingDataTest, SizeMatchesSizeof)
{
    EXPECT_EQ(SceneLightingData::Size(), sizeof(SceneLightingData));
}

TEST(SceneLightingDataTest, MaxLightConstants)
{
    EXPECT_EQ(MAX_POINT_LIGHTS, 16);
    EXPECT_EQ(MAX_SPOT_LIGHTS, 8);
}

TEST(SceneLightingDataTest, PointLightGPULayout)
{
    PointLightGPU point{};
    point.positionAndRange = glm::vec4(1.0f, 2.0f, 3.0f, 10.0f);
    point.colorAndIntensity = glm::vec4(1.0f, 0.5f, 0.0f, 2.0f);

    EXPECT_FLOAT_EQ(point.positionAndRange.x, 1.0f);
    EXPECT_FLOAT_EQ(point.positionAndRange.y, 2.0f);
    EXPECT_FLOAT_EQ(point.positionAndRange.z, 3.0f);
    EXPECT_FLOAT_EQ(point.positionAndRange.w, 10.0f);
    EXPECT_FLOAT_EQ(point.colorAndIntensity.w, 2.0f);
}

TEST(SceneLightingDataTest, SpotLightGPULayout)
{
    SpotLightGPU spot{};
    spot.positionAndRange = glm::vec4(1.0f, 2.0f, 3.0f, 15.0f);
    spot.directionAndAngle = glm::vec4(0.0f, -1.0f, 0.0f, 0.707f);
    spot.colorAndIntensity = glm::vec4(1.0f, 1.0f, 1.0f, 3.0f);
    spot.spotParams = glm::vec4(0.866f, 0.0f, 0.0f, 0.0f);

    EXPECT_FLOAT_EQ(spot.positionAndRange.w, 15.0f);
    EXPECT_FLOAT_EQ(spot.directionAndAngle.y, -1.0f);
    EXPECT_NEAR(spot.directionAndAngle.w, 0.707f, 0.001f);
    EXPECT_FLOAT_EQ(spot.colorAndIntensity.w, 3.0f);
    EXPECT_NEAR(spot.spotParams.x, 0.866f, 0.001f);
}

TEST(SceneLightingDataTest, SpotLightGPUSize)
{
    EXPECT_EQ(sizeof(SpotLightGPU), 64u);
}

TEST(SceneLightingDataTest, SceneLightingDataTotalSize)
{
    EXPECT_EQ(SceneLightingData::Size(), 1184u);
}

TEST(SceneLightingDataTest, FieldAssignment)
{
    SceneLightingData data{};
    data.cameraPosition = glm::vec4(10.0f, 20.0f, 30.0f, 1.0f);
    data.ambientColor = glm::vec4(0.15f, 0.15f, 0.15f, 1.0f);
    data.directionalDirection = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
    data.directionalColor = glm::vec4(1.0f, 1.0f, 0.9f, 1.5f);
    data.pointLightCount = 3;
    data.spotLightCount = 1;

    data.pointLights[0].positionAndRange = glm::vec4(5.0f, 5.0f, 5.0f, 10.0f);
    data.pointLights[0].colorAndIntensity = glm::vec4(1.0f, 0.0f, 0.0f, 2.0f);

    EXPECT_FLOAT_EQ(data.cameraPosition.x, 10.0f);
    EXPECT_FLOAT_EQ(data.ambientColor.w, 1.0f);
    EXPECT_FLOAT_EQ(data.directionalColor.w, 1.5f);
    EXPECT_EQ(data.pointLightCount, 3);
    EXPECT_EQ(data.spotLightCount, 1);
    EXPECT_FLOAT_EQ(data.pointLights[0].colorAndIntensity.r, 1.0f);
}

TEST(SceneLightingDataTest, PaddingFieldsExist)
{
    SceneLightingData data{};
    EXPECT_EQ(data._pad0, 0);
    EXPECT_EQ(data._pad1, 0);
}

TEST(SceneLightingDataTest, DirectionalLightVPDefault)
{
    SceneLightingData data{};
    EXPECT_FLOAT_EQ(data.directionalLightVP[0][0], 1.0f);
    EXPECT_FLOAT_EQ(data.directionalLightVP[1][1], 1.0f);
    EXPECT_FLOAT_EQ(data.directionalLightVP[2][2], 1.0f);
    EXPECT_FLOAT_EQ(data.directionalLightVP[3][3], 1.0f);
    EXPECT_FLOAT_EQ(data.directionalLightVP[0][1], 0.0f);
}

TEST(SceneLightingDataTest, ShadowParamsDefault)
{
    SceneLightingData data{};
    EXPECT_FLOAT_EQ(data.shadowParams.x, 0.0f);
    EXPECT_FLOAT_EQ(data.shadowParams.y, 0.0f);
    EXPECT_FLOAT_EQ(data.shadowParams.z, 0.0f);
    EXPECT_FLOAT_EQ(data.shadowParams.w, 0.0f);
}

TEST(SceneLightingDataTest, DirectionalLightVPAssignment)
{
    SceneLightingData data{};
    glm::mat4 testVP(2.0f);
    data.directionalLightVP = testVP;
    EXPECT_FLOAT_EQ(data.directionalLightVP[0][0], 2.0f);
    EXPECT_FLOAT_EQ(data.directionalLightVP[1][1], 2.0f);
}

TEST(SceneLightingDataTest, ShadowParamsAssignment)
{
    SceneLightingData data{};
    data.shadowParams = glm::vec4(1.0f, 0.005f, 2048.0f, 0.0f);
    EXPECT_FLOAT_EQ(data.shadowParams.x, 1.0f);
    EXPECT_FLOAT_EQ(data.shadowParams.y, 0.005f);
    EXPECT_FLOAT_EQ(data.shadowParams.z, 2048.0f);
}

TEST(SceneLightingDataTest, Vec4Alignment)
{
    EXPECT_EQ(offsetof(SceneLightingData, cameraPosition) % 16, 0u);
    EXPECT_EQ(offsetof(SceneLightingData, ambientColor) % 16, 0u);
    EXPECT_EQ(offsetof(SceneLightingData, directionalDirection) % 16, 0u);
    EXPECT_EQ(offsetof(SceneLightingData, directionalColor) % 16, 0u);
    EXPECT_EQ(offsetof(SceneLightingData, pointLightCount) % 16, 0u);
    EXPECT_EQ(offsetof(SceneLightingData, directionalLightVP) % 16, 0u);
    EXPECT_EQ(offsetof(SceneLightingData, shadowParams) % 16, 0u);
}
