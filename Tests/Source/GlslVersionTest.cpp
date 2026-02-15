#include <gtest/gtest.h>
#include "RenderStar/Client/Render/Shader/GlslVersion.hpp"

using namespace RenderStar::Client::Render::Shader;

TEST(GlslVersionTest, GetVersionNumber330)
{
    EXPECT_EQ(GetVersionNumber(GlslVersion::GLSL_330), 330);
}

TEST(GlslVersionTest, GetVersionNumber410)
{
    EXPECT_EQ(GetVersionNumber(GlslVersion::GLSL_410), 410);
}

TEST(GlslVersionTest, GetVersionNumber450)
{
    EXPECT_EQ(GetVersionNumber(GlslVersion::GLSL_450), 450);
}

TEST(GlslVersionTest, GetVersionNumber460)
{
    EXPECT_EQ(GetVersionNumber(GlslVersion::GLSL_460), 460);
}

TEST(GlslVersionTest, GetVersionString330)
{
    EXPECT_EQ(GetVersionString(GlslVersion::GLSL_330), "330 core");
}

TEST(GlslVersionTest, GetVersionString450)
{
    EXPECT_EQ(GetVersionString(GlslVersion::GLSL_450), "450");
}

TEST(GlslVersionTest, IsOpenGLCompatible)
{
    EXPECT_TRUE(IsOpenGLCompatible(GlslVersion::GLSL_330));
    EXPECT_TRUE(IsOpenGLCompatible(GlslVersion::GLSL_410));
    EXPECT_FALSE(IsOpenGLCompatible(GlslVersion::GLSL_450));
    EXPECT_TRUE(IsOpenGLCompatible(GlslVersion::GLSL_460));
}

TEST(GlslVersionTest, GetVersionDirective)
{
    EXPECT_EQ(GetVersionDirective(GlslVersion::GLSL_410), "#version 410 core");
    EXPECT_EQ(GetVersionDirective(GlslVersion::GLSL_450), "#version 450");
}

TEST(GlslVersionTest, ParseVersionFromSource450)
{
    auto version = ParseVersionFromSource("#version 450\nvoid main() {}");
    ASSERT_TRUE(version.has_value());
    EXPECT_EQ(*version, GlslVersion::GLSL_450);
}

TEST(GlslVersionTest, ParseVersionFromSourceEmpty)
{
    EXPECT_FALSE(ParseVersionFromSource("").has_value());
}

TEST(GlslVersionTest, ParseVersionFromSourceNoDirective)
{
    EXPECT_FALSE(ParseVersionFromSource("void main() {}").has_value());
}

TEST(GlslVersionTest, ParseVersionFromSource330)
{
    auto version = ParseVersionFromSource("#version 330 core\n");
    ASSERT_TRUE(version.has_value());
    EXPECT_EQ(*version, GlslVersion::GLSL_330);
}
