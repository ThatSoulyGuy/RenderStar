#include <gtest/gtest.h>
#include "RenderStar/Client/Render/Framework/LitVertex.hpp"

using namespace RenderStar::Client::Render;
using namespace RenderStar::Client::Render::Framework;

TEST(LitVertexTest, DefaultConstructor)
{
    LitVertex v;
    EXPECT_FLOAT_EQ(v.posX, 0.0f);
    EXPECT_FLOAT_EQ(v.posY, 0.0f);
    EXPECT_FLOAT_EQ(v.posZ, 0.0f);
    EXPECT_FLOAT_EQ(v.normalX, 0.0f);
    EXPECT_FLOAT_EQ(v.normalY, 1.0f);
    EXPECT_FLOAT_EQ(v.normalZ, 0.0f);
    EXPECT_FLOAT_EQ(v.texU, 0.0f);
    EXPECT_FLOAT_EQ(v.texV, 0.0f);
    EXPECT_FLOAT_EQ(v.tangentX, 1.0f);
    EXPECT_FLOAT_EQ(v.tangentY, 0.0f);
    EXPECT_FLOAT_EQ(v.tangentZ, 0.0f);
}

TEST(LitVertexTest, ParameterizedConstructor)
{
    LitVertex v(1.0f, 2.0f, 3.0f, 0.0f, 0.0f, 1.0f, 0.5f, 0.75f);
    EXPECT_FLOAT_EQ(v.posX, 1.0f);
    EXPECT_FLOAT_EQ(v.posY, 2.0f);
    EXPECT_FLOAT_EQ(v.posZ, 3.0f);
    EXPECT_FLOAT_EQ(v.normalX, 0.0f);
    EXPECT_FLOAT_EQ(v.normalY, 0.0f);
    EXPECT_FLOAT_EQ(v.normalZ, 1.0f);
    EXPECT_FLOAT_EQ(v.texU, 0.5f);
    EXPECT_FLOAT_EQ(v.texV, 0.75f);
    EXPECT_FLOAT_EQ(v.tangentX, 1.0f);
    EXPECT_FLOAT_EQ(v.tangentY, 0.0f);
    EXPECT_FLOAT_EQ(v.tangentZ, 0.0f);
}

TEST(LitVertexTest, FullConstructor)
{
    LitVertex v(1.0f, 2.0f, 3.0f, 0.0f, 0.0f, 1.0f, 0.5f, 0.75f, 0.0f, 1.0f, 0.0f);
    EXPECT_FLOAT_EQ(v.tangentX, 0.0f);
    EXPECT_FLOAT_EQ(v.tangentY, 1.0f);
    EXPECT_FLOAT_EQ(v.tangentZ, 0.0f);
}

TEST(LitVertexTest, LayoutAttributeCount)
{
    LitVertex v;
    const auto& layout = v.GetLayout();
    EXPECT_EQ(layout.attributes.size(), 4);
}

TEST(LitVertexTest, LayoutStride)
{
    EXPECT_EQ(LitVertex::LAYOUT.stride, static_cast<int32_t>(sizeof(float) * 11));
}

TEST(LitVertexTest, LayoutLocations)
{
    const auto& attrs = LitVertex::LAYOUT.attributes;
    ASSERT_EQ(attrs.size(), 4u);
    EXPECT_EQ(attrs[0].location, 0u);
    EXPECT_EQ(attrs[1].location, 1u);
    EXPECT_EQ(attrs[2].location, 2u);
    EXPECT_EQ(attrs[3].location, 3u);
}

TEST(LitVertexTest, LayoutTypes)
{
    const auto& attrs = LitVertex::LAYOUT.attributes;
    EXPECT_EQ(attrs[0].type, VertexAttributeType::FLOAT3);
    EXPECT_EQ(attrs[1].type, VertexAttributeType::FLOAT3);
    EXPECT_EQ(attrs[2].type, VertexAttributeType::FLOAT2);
    EXPECT_EQ(attrs[3].type, VertexAttributeType::FLOAT3);
}

TEST(LitVertexTest, LayoutOffsets)
{
    const auto& attrs = LitVertex::LAYOUT.attributes;
    EXPECT_EQ(attrs[0].offset, 0);
    EXPECT_EQ(attrs[1].offset, static_cast<int32_t>(sizeof(float) * 3));
    EXPECT_EQ(attrs[2].offset, static_cast<int32_t>(sizeof(float) * 6));
    EXPECT_EQ(attrs[3].offset, static_cast<int32_t>(sizeof(float) * 8));
}

TEST(LitVertexTest, WriteTo)
{
    LitVertex v(1.0f, 2.0f, 3.0f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 0.1f, 0.2f, 0.3f);
    std::vector<float> data(16, 0.0f);
    v.WriteTo(data, 0);

    EXPECT_FLOAT_EQ(data[0], 1.0f);
    EXPECT_FLOAT_EQ(data[1], 2.0f);
    EXPECT_FLOAT_EQ(data[2], 3.0f);
    EXPECT_FLOAT_EQ(data[3], 0.5f);
    EXPECT_FLOAT_EQ(data[4], 0.6f);
    EXPECT_FLOAT_EQ(data[5], 0.7f);
    EXPECT_FLOAT_EQ(data[6], 0.8f);
    EXPECT_FLOAT_EQ(data[7], 0.9f);
    EXPECT_FLOAT_EQ(data[8], 0.1f);
    EXPECT_FLOAT_EQ(data[9], 0.2f);
    EXPECT_FLOAT_EQ(data[10], 0.3f);
}

TEST(LitVertexTest, WriteToWithOffset)
{
    LitVertex v(1.0f, 2.0f, 3.0f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 0.1f, 0.2f, 0.3f);
    std::vector<float> data(22, 0.0f);
    v.WriteTo(data, 11);

    EXPECT_FLOAT_EQ(data[11], 1.0f);
    EXPECT_FLOAT_EQ(data[12], 2.0f);
    EXPECT_FLOAT_EQ(data[19], 0.1f);
    EXPECT_FLOAT_EQ(data[20], 0.2f);
    EXPECT_FLOAT_EQ(data[21], 0.3f);
}

TEST(LitVertexTest, StaticLayoutMatchesInstance)
{
    LitVertex v;
    EXPECT_EQ(LitVertex::LAYOUT.stride, v.GetLayout().stride);
    EXPECT_EQ(LitVertex::LAYOUT.attributes.size(), v.GetLayout().attributes.size());
}
