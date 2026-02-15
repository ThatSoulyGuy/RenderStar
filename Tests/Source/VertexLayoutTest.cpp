#include <gtest/gtest.h>
#include "RenderStar/Client/Render/Resource/VertexLayout.hpp"

using namespace RenderStar::Client::Render;

TEST(VertexLayoutTest, PositionNormalUV)
{
    auto layout = VertexLayout::PositionNormalUV();
    EXPECT_EQ(layout.attributes.size(), 3);
    EXPECT_EQ(layout.stride, static_cast<int32_t>(sizeof(float) * 8));
}

TEST(VertexLayoutTest, PositionColor)
{
    auto layout = VertexLayout::PositionColor();
    EXPECT_EQ(layout.attributes.size(), 2);
    EXPECT_EQ(layout.stride, static_cast<int32_t>(sizeof(float) * 7));
}

TEST(VertexLayoutTest, PositionOnly)
{
    auto layout = VertexLayout::PositionOnly();
    EXPECT_EQ(layout.attributes.size(), 1);
    EXPECT_EQ(layout.stride, static_cast<int32_t>(sizeof(float) * 3));
}

TEST(VertexLayoutTest, ComponentCountFloat)
{
    EXPECT_EQ(VertexAttribute::GetComponentCount(VertexAttributeType::FLOAT), 1);
    EXPECT_EQ(VertexAttribute::GetComponentCount(VertexAttributeType::FLOAT2), 2);
    EXPECT_EQ(VertexAttribute::GetComponentCount(VertexAttributeType::FLOAT3), 3);
    EXPECT_EQ(VertexAttribute::GetComponentCount(VertexAttributeType::FLOAT4), 4);
}

TEST(VertexLayoutTest, ComponentCountInt)
{
    EXPECT_EQ(VertexAttribute::GetComponentCount(VertexAttributeType::INT), 1);
    EXPECT_EQ(VertexAttribute::GetComponentCount(VertexAttributeType::INT2), 2);
    EXPECT_EQ(VertexAttribute::GetComponentCount(VertexAttributeType::INT3), 3);
    EXPECT_EQ(VertexAttribute::GetComponentCount(VertexAttributeType::INT4), 4);
}

TEST(VertexLayoutTest, GetSizeFloat)
{
    EXPECT_EQ(VertexAttribute::GetSize(VertexAttributeType::FLOAT), sizeof(float));
    EXPECT_EQ(VertexAttribute::GetSize(VertexAttributeType::FLOAT2), sizeof(float) * 2);
    EXPECT_EQ(VertexAttribute::GetSize(VertexAttributeType::FLOAT3), sizeof(float) * 3);
    EXPECT_EQ(VertexAttribute::GetSize(VertexAttributeType::FLOAT4), sizeof(float) * 4);
}

TEST(VertexLayoutTest, GetSizeInt)
{
    EXPECT_EQ(VertexAttribute::GetSize(VertexAttributeType::INT), sizeof(int32_t));
    EXPECT_EQ(VertexAttribute::GetSize(VertexAttributeType::UINT), sizeof(uint32_t));
}

TEST(VertexLayoutTest, AttributeOffsets)
{
    auto layout = VertexLayout::PositionNormalUV();
    EXPECT_EQ(layout.attributes[0].offset, 0);
    EXPECT_EQ(layout.attributes[1].offset, sizeof(float) * 3);
    EXPECT_EQ(layout.attributes[2].offset, sizeof(float) * 6);
}

TEST(VertexLayoutTest, AttributeLocations)
{
    auto layout = VertexLayout::PositionNormalUV();
    EXPECT_EQ(layout.attributes[0].location, 0u);
    EXPECT_EQ(layout.attributes[1].location, 1u);
    EXPECT_EQ(layout.attributes[2].location, 2u);
}
