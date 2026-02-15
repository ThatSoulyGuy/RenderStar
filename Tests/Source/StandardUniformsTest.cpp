#include <gtest/gtest.h>
#include "RenderStar/Client/Render/Resource/StandardUniforms.hpp"

using namespace RenderStar::Client::Render;

TEST(StandardUniformsTest, DefaultModel)
{
    StandardUniforms u;
    EXPECT_EQ(u.model, glm::mat4(1.0f));
}

TEST(StandardUniformsTest, DefaultViewProjection)
{
    StandardUniforms u;
    EXPECT_EQ(u.viewProjection, glm::mat4(1.0f));
}

TEST(StandardUniformsTest, DefaultColorTint)
{
    StandardUniforms u;
    EXPECT_FLOAT_EQ(u.colorTint.r, 1.0f);
    EXPECT_FLOAT_EQ(u.colorTint.g, 1.0f);
    EXPECT_FLOAT_EQ(u.colorTint.b, 1.0f);
    EXPECT_FLOAT_EQ(u.colorTint.a, 0.0f);
}

TEST(StandardUniformsTest, ParameterizedConstructor)
{
    glm::mat4 model(2.0f);
    glm::mat4 vp(3.0f);
    glm::vec4 tint(0.5f, 0.5f, 0.5f, 1.0f);

    StandardUniforms u(model, vp, tint);
    EXPECT_EQ(u.model, model);
    EXPECT_EQ(u.viewProjection, vp);
    EXPECT_EQ(u.colorTint, tint);
}

TEST(StandardUniformsTest, Size)
{
    EXPECT_EQ(StandardUniforms::Size(), sizeof(StandardUniforms));
}
