#include <gtest/gtest.h>
#include "RenderStar/Client/Render/Components/Light.hpp"

using namespace RenderStar::Client::Render::Components;

TEST(LightComponentTest, DefaultValues)
{
    Light light;

    EXPECT_EQ(light.type, LightType::DIRECTIONAL);
    EXPECT_FLOAT_EQ(light.color.r, 1.0f);
    EXPECT_FLOAT_EQ(light.color.g, 1.0f);
    EXPECT_FLOAT_EQ(light.color.b, 1.0f);
    EXPECT_FLOAT_EQ(light.intensity, 1.0f);
    EXPECT_FLOAT_EQ(light.direction.x, 0.0f);
    EXPECT_FLOAT_EQ(light.direction.y, -1.0f);
    EXPECT_FLOAT_EQ(light.direction.z, 0.0f);
    EXPECT_FLOAT_EQ(light.range, 10.0f);
    EXPECT_FLOAT_EQ(light.spotAngle, 45.0f);
    EXPECT_FLOAT_EQ(light.spotSoftness, 0.1f);
    EXPECT_FALSE(light.castShadows);
}

TEST(LightComponentTest, DirectionalFactory)
{
    auto light = Light::Directional(glm::vec3(0.0f, -1.0f, 0.5f), glm::vec3(1.0f, 0.8f, 0.6f), 2.0f);

    EXPECT_EQ(light.type, LightType::DIRECTIONAL);
    EXPECT_FLOAT_EQ(light.direction.x, 0.0f);
    EXPECT_FLOAT_EQ(light.direction.y, -1.0f);
    EXPECT_FLOAT_EQ(light.direction.z, 0.5f);
    EXPECT_FLOAT_EQ(light.color.r, 1.0f);
    EXPECT_FLOAT_EQ(light.color.g, 0.8f);
    EXPECT_FLOAT_EQ(light.color.b, 0.6f);
    EXPECT_FLOAT_EQ(light.intensity, 2.0f);
}

TEST(LightComponentTest, PointFactory)
{
    auto light = Light::Point(glm::vec3(0.5f, 0.5f, 1.0f), 3.0f, 25.0f);

    EXPECT_EQ(light.type, LightType::POINT);
    EXPECT_FLOAT_EQ(light.color.r, 0.5f);
    EXPECT_FLOAT_EQ(light.color.g, 0.5f);
    EXPECT_FLOAT_EQ(light.color.b, 1.0f);
    EXPECT_FLOAT_EQ(light.intensity, 3.0f);
    EXPECT_FLOAT_EQ(light.range, 25.0f);
}

TEST(LightComponentTest, SpotFactory)
{
    auto light = Light::Spot(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(1.0f), 1.5f, 15.0f, 30.0f);

    EXPECT_EQ(light.type, LightType::SPOT);
    EXPECT_FLOAT_EQ(light.direction.x, 0.0f);
    EXPECT_FLOAT_EQ(light.direction.y, -1.0f);
    EXPECT_FLOAT_EQ(light.direction.z, 0.0f);
    EXPECT_FLOAT_EQ(light.color.r, 1.0f);
    EXPECT_FLOAT_EQ(light.intensity, 1.5f);
    EXPECT_FLOAT_EQ(light.range, 15.0f);
    EXPECT_FLOAT_EQ(light.spotAngle, 30.0f);
}

TEST(LightComponentTest, LightTypeEnumValues)
{
    EXPECT_EQ(static_cast<uint32_t>(LightType::DIRECTIONAL), 0u);
    EXPECT_EQ(static_cast<uint32_t>(LightType::POINT), 1u);
    EXPECT_EQ(static_cast<uint32_t>(LightType::SPOT), 2u);
}
