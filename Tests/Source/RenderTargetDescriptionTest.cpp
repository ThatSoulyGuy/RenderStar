#include <gtest/gtest.h>
#include "RenderStar/Client/Render/Platform/IRenderTarget.hpp"

using namespace RenderStar::Client::Render;
using namespace RenderStar::Client::Render::Platform;

TEST(RenderTargetDescriptionTest, DefaultDescription)
{
    RenderTargetDescription desc;
    EXPECT_TRUE(desc.name.empty());
    EXPECT_EQ(desc.width, 0u);
    EXPECT_EQ(desc.height, 0u);
    EXPECT_EQ(desc.colorFormat, TextureFormat::RGBA8);
    EXPECT_FALSE(desc.hasDepth);
    EXPECT_TRUE(desc.matchSwapchainSize);
    EXPECT_FLOAT_EQ(desc.scale, 1.0f);
}

TEST(RenderTargetDescriptionTest, CustomDescription)
{
    RenderTargetDescription desc;
    desc.name = "hdr_buffer";
    desc.width = 1920;
    desc.height = 1080;
    desc.colorFormat = TextureFormat::RGBA16F;
    desc.hasDepth = true;
    desc.matchSwapchainSize = false;

    EXPECT_EQ(desc.name, "hdr_buffer");
    EXPECT_EQ(desc.width, 1920u);
    EXPECT_EQ(desc.height, 1080u);
    EXPECT_EQ(desc.colorFormat, TextureFormat::RGBA16F);
    EXPECT_TRUE(desc.hasDepth);
    EXPECT_FALSE(desc.matchSwapchainSize);
}

TEST(RenderTargetDescriptionTest, TextureFormatValues)
{
    EXPECT_NE(TextureFormat::RGBA8, TextureFormat::RGBA16F);
    EXPECT_NE(TextureFormat::RGBA8, TextureFormat::RGBA32F);
    EXPECT_NE(TextureFormat::RGBA16F, TextureFormat::RGBA32F);
    EXPECT_NE(TextureFormat::RGBA8, TextureFormat::DEPTH24_STENCIL8);
    EXPECT_NE(TextureFormat::RGBA8, TextureFormat::DEPTH32F);
    EXPECT_NE(TextureFormat::DEPTH24_STENCIL8, TextureFormat::DEPTH32F);
}

TEST(RenderTargetDescriptionTest, RenderTargetTypeValues)
{
    EXPECT_NE(RenderTargetType::COLOR_DEPTH, RenderTargetType::COLOR_ONLY);
    EXPECT_NE(RenderTargetType::COLOR_DEPTH, RenderTargetType::DEPTH_ONLY);
    EXPECT_NE(RenderTargetType::COLOR_DEPTH, RenderTargetType::SWAPCHAIN);
    EXPECT_NE(RenderTargetType::COLOR_ONLY, RenderTargetType::DEPTH_ONLY);
    EXPECT_NE(RenderTargetType::COLOR_ONLY, RenderTargetType::SWAPCHAIN);
    EXPECT_NE(RenderTargetType::DEPTH_ONLY, RenderTargetType::SWAPCHAIN);
}
