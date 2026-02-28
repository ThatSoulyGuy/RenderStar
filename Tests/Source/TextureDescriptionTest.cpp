#include <gtest/gtest.h>
#include "RenderStar/Client/Render/Resource/ITextureHandle.hpp"

using namespace RenderStar::Client::Render;

TEST(TextureDescriptionTest, DefaultValues)
{
    TextureDescription desc;
    EXPECT_EQ(desc.width, 1u);
    EXPECT_EQ(desc.height, 1u);
    EXPECT_EQ(desc.format, TextureFormat::RGBA8);
    EXPECT_EQ(desc.wrapS, TextureWrapMode::REPEAT);
    EXPECT_EQ(desc.wrapT, TextureWrapMode::REPEAT);
    EXPECT_EQ(desc.minFilter, TextureFilterMode::LINEAR);
    EXPECT_EQ(desc.magFilter, TextureFilterMode::LINEAR);
}

TEST(TextureDescriptionTest, CustomValues)
{
    TextureDescription desc;
    desc.width = 512;
    desc.height = 256;
    desc.wrapS = TextureWrapMode::CLAMP_TO_EDGE;
    desc.wrapT = TextureWrapMode::CLAMP_TO_EDGE;
    desc.minFilter = TextureFilterMode::NEAREST;
    desc.magFilter = TextureFilterMode::NEAREST;

    EXPECT_EQ(desc.width, 512u);
    EXPECT_EQ(desc.height, 256u);
    EXPECT_EQ(desc.wrapS, TextureWrapMode::CLAMP_TO_EDGE);
    EXPECT_EQ(desc.wrapT, TextureWrapMode::CLAMP_TO_EDGE);
    EXPECT_EQ(desc.minFilter, TextureFilterMode::NEAREST);
    EXPECT_EQ(desc.magFilter, TextureFilterMode::NEAREST);
}

TEST(TextureDescriptionTest, WrapModeEnumValues)
{
    EXPECT_NE(TextureWrapMode::REPEAT, TextureWrapMode::CLAMP_TO_EDGE);
}

TEST(TextureDescriptionTest, FilterModeEnumValues)
{
    EXPECT_NE(TextureFilterMode::NEAREST, TextureFilterMode::LINEAR);
}

TEST(TextureDescriptionTest, MixedWrapAndFilter)
{
    TextureDescription desc;
    desc.wrapS = TextureWrapMode::REPEAT;
    desc.wrapT = TextureWrapMode::CLAMP_TO_EDGE;
    desc.minFilter = TextureFilterMode::LINEAR;
    desc.magFilter = TextureFilterMode::NEAREST;

    EXPECT_NE(desc.wrapS, desc.wrapT);
    EXPECT_NE(desc.minFilter, desc.magFilter);
}
