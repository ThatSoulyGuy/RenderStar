#include <gtest/gtest.h>
#include "RenderStar/Client/Render/Resource/UniformLayout.hpp"

using namespace RenderStar::Client::Render;

TEST(UniformLayoutTest, ForMVPBinding)
{
    auto layout = UniformLayout::ForMVP();
    EXPECT_EQ(layout.GetBindingCount(), 1);
    EXPECT_EQ(layout.GetBinding(0).binding, 0u);
    EXPECT_EQ(layout.GetBinding(0).type, UniformType::UNIFORM_BUFFER);
    EXPECT_EQ(layout.GetBinding(0).stages, ShaderStage::VERTEX);
}

TEST(UniformLayoutTest, ForMVPName)
{
    auto layout = UniformLayout::ForMVP();
    EXPECT_EQ(layout.GetBinding(0).name, "UniformBufferObject");
}

TEST(UniformLayoutTest, ForMVPSize)
{
    auto layout = UniformLayout::ForMVP();
    EXPECT_EQ(layout.GetBinding(0).size, 3 * 16 * sizeof(float));
}

TEST(UniformLayoutTest, EmptyLayout)
{
    auto layout = UniformLayout::Empty();
    EXPECT_EQ(layout.GetBindingCount(), 0);
}

TEST(UniformLayoutTest, CustomLayout)
{
    UniformLayout layout;
    layout.bindings.push_back(UniformBinding{
        0, "TestBuffer", UniformType::STORAGE_BUFFER, 256, ShaderStage::COMPUTE
    });
    layout.bindings.push_back(UniformBinding{
        1, "Sampler", UniformType::COMBINED_IMAGE_SAMPLER, 0, ShaderStage::FRAGMENT
    });

    EXPECT_EQ(layout.GetBindingCount(), 2);
    EXPECT_EQ(layout.GetBinding(0).name, "TestBuffer");
    EXPECT_EQ(layout.GetBinding(1).type, UniformType::COMBINED_IMAGE_SAMPLER);
}

TEST(UniformLayoutTest, ShaderStageValues)
{
    EXPECT_NE(ShaderStage::VERTEX, ShaderStage::FRAGMENT);
    EXPECT_NE(ShaderStage::VERTEX, ShaderStage::COMPUTE);
}
