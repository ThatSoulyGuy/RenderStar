#include <gtest/gtest.h>
#include "RenderStar/Client/Render/Backend/BackendCapabilities.hpp"

using namespace RenderStar::Client::Render;

TEST(BackendCapabilitiesTest, ForVulkan)
{
    auto caps = BackendCapabilities::ForVulkan();
    EXPECT_TRUE(caps.supportsCompute);
    EXPECT_TRUE(caps.supportsMultiDrawIndirect);
    EXPECT_TRUE(caps.supportsBindlessTextures);
    EXPECT_EQ(caps.maxUniformBufferSize, 65536);
    EXPECT_EQ(caps.maxTextureSize, 16384);
    EXPECT_EQ(caps.maxFramesInFlight, 2);
}

TEST(BackendCapabilitiesTest, ForOpenGL33)
{
    auto caps = BackendCapabilities::ForOpenGL33();
    EXPECT_FALSE(caps.supportsCompute);
    EXPECT_FALSE(caps.supportsMultiDrawIndirect);
    EXPECT_FALSE(caps.supportsBindlessTextures);
    EXPECT_EQ(caps.maxUniformBufferSize, 16384);
    EXPECT_EQ(caps.maxTextureSize, 16384);
    EXPECT_EQ(caps.maxFramesInFlight, 2);
}

TEST(BackendCapabilitiesTest, ForOpenGL45)
{
    auto caps = BackendCapabilities::ForOpenGL45();
    EXPECT_TRUE(caps.supportsCompute);
    EXPECT_TRUE(caps.supportsMultiDrawIndirect);
    EXPECT_TRUE(caps.supportsBindlessTextures);
    EXPECT_EQ(caps.maxUniformBufferSize, 65536);
}

TEST(BackendCapabilitiesTest, AllFactoriesMaxFramesInFlight)
{
    EXPECT_EQ(BackendCapabilities::ForVulkan().maxFramesInFlight, 2);
    EXPECT_EQ(BackendCapabilities::ForOpenGL33().maxFramesInFlight, 2);
    EXPECT_EQ(BackendCapabilities::ForOpenGL45().maxFramesInFlight, 2);
}

TEST(BackendCapabilitiesTest, VulkanVsOpenGL33Differences)
{
    auto vk = BackendCapabilities::ForVulkan();
    auto gl33 = BackendCapabilities::ForOpenGL33();

    EXPECT_NE(vk.supportsCompute, gl33.supportsCompute);
    EXPECT_GT(vk.maxUniformBufferSize, gl33.maxUniformBufferSize);
}
