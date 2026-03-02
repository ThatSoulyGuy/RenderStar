#include <gtest/gtest.h>
#include "RenderStar/Client/Render/Resource/MaterialProperties.hpp"

using namespace RenderStar::Client::Render;

TEST(MaterialPropertiesTest, DefaultParams)
{
    MaterialProperties m;
    EXPECT_FLOAT_EQ(m.params.x, 0.5f);
    EXPECT_FLOAT_EQ(m.params.y, 0.0f);
    EXPECT_FLOAT_EQ(m.params.z, 1.0f);
    EXPECT_FLOAT_EQ(m.params.w, 0.0f);
    EXPECT_FLOAT_EQ(m.params2.x, 1.0f);
    EXPECT_FLOAT_EQ(m.params2.y, 0.5f);
    EXPECT_FLOAT_EQ(m.params2.z, 0.0f);
    EXPECT_FLOAT_EQ(m.params2.w, 0.0f);
}

TEST(MaterialPropertiesTest, ParameterizedConstructor)
{
    MaterialProperties m(0.8f, 1.0f, 0.5f, 0.3f);
    EXPECT_FLOAT_EQ(m.params.x, 0.8f);
    EXPECT_FLOAT_EQ(m.params.y, 1.0f);
    EXPECT_FLOAT_EQ(m.params.z, 0.5f);
    EXPECT_FLOAT_EQ(m.params.w, 0.3f);
    EXPECT_FLOAT_EQ(m.params2.x, 1.0f);
}

TEST(MaterialPropertiesTest, ParameterizedConstructorWithNormalStrength)
{
    MaterialProperties m(0.8f, 1.0f, 0.5f, 0.3f, 0.7f);
    EXPECT_FLOAT_EQ(m.params.x, 0.8f);
    EXPECT_FLOAT_EQ(m.params2.x, 0.7f);
}

TEST(MaterialPropertiesTest, Size)
{
    EXPECT_EQ(MaterialProperties::Size(), sizeof(MaterialProperties));
}

TEST(MaterialPropertiesTest, SizeIs32Bytes)
{
    EXPECT_EQ(MaterialProperties::Size(), 32u);
}

TEST(MaterialPropertiesTest, Std140Alignment)
{
    EXPECT_EQ(MaterialProperties::Size() % 16, 0u);
}
