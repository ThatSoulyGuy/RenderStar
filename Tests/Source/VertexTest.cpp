#include <gtest/gtest.h>
#include "RenderStar/Client/Render/Resource/Vertex.hpp"

using namespace RenderStar::Client::Render;

TEST(VertexTest, DefaultConstructor)
{
    Vertex v;
    EXPECT_FLOAT_EQ(v.posX, 0.0f);
    EXPECT_FLOAT_EQ(v.posY, 0.0f);
    EXPECT_FLOAT_EQ(v.posZ, 0.0f);
    EXPECT_FLOAT_EQ(v.colorR, 1.0f);
    EXPECT_FLOAT_EQ(v.colorG, 1.0f);
    EXPECT_FLOAT_EQ(v.colorB, 1.0f);
    EXPECT_FLOAT_EQ(v.texU, 0.0f);
    EXPECT_FLOAT_EQ(v.texV, 0.0f);
}

TEST(VertexTest, ParameterizedConstructor)
{
    Vertex v(1.0f, 2.0f, 3.0f, 0.5f, 0.6f, 0.7f, 0.1f, 0.2f);
    EXPECT_FLOAT_EQ(v.posX, 1.0f);
    EXPECT_FLOAT_EQ(v.posY, 2.0f);
    EXPECT_FLOAT_EQ(v.posZ, 3.0f);
    EXPECT_FLOAT_EQ(v.colorR, 0.5f);
    EXPECT_FLOAT_EQ(v.colorG, 0.6f);
    EXPECT_FLOAT_EQ(v.colorB, 0.7f);
    EXPECT_FLOAT_EQ(v.texU, 0.1f);
    EXPECT_FLOAT_EQ(v.texV, 0.2f);
}

TEST(VertexTest, GetLayoutAttributes)
{
    Vertex v;
    const auto& layout = v.GetLayout();
    EXPECT_EQ(layout.attributes.size(), 3);
    EXPECT_EQ(layout.stride, static_cast<int32_t>(sizeof(float) * 8));
}

TEST(VertexTest, WriteTo)
{
    Vertex v(1.0f, 2.0f, 3.0f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f);
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
}

TEST(VertexTest, WriteToWithOffset)
{
    Vertex v(1.0f, 2.0f, 3.0f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f);
    std::vector<float> data(16, 0.0f);
    v.WriteTo(data, 8);

    EXPECT_FLOAT_EQ(data[8], 1.0f);
    EXPECT_FLOAT_EQ(data[9], 2.0f);
}

TEST(VertexTest, ToFloatArraySingle)
{
    std::vector<Vertex> vertices = { Vertex(1.0f, 2.0f, 3.0f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f) };
    auto floats = Vertex::ToFloatArray(std::span<const Vertex>(vertices));

    ASSERT_EQ(floats.size(), 8);
    EXPECT_FLOAT_EQ(floats[0], 1.0f);
    EXPECT_FLOAT_EQ(floats[7], 0.8f);
}

TEST(VertexTest, ToFloatArrayMultiple)
{
    std::vector<Vertex> vertices = {
        Vertex(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f),
        Vertex(2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f)
    };
    auto floats = Vertex::ToFloatArray(std::span<const Vertex>(vertices));

    ASSERT_EQ(floats.size(), 16);
    EXPECT_FLOAT_EQ(floats[0], 1.0f);
    EXPECT_FLOAT_EQ(floats[8], 2.0f);
}

TEST(VertexTest, ToFloatArrayEmpty)
{
    std::vector<Vertex> empty;
    auto floats = Vertex::ToFloatArray(std::span<const Vertex>(empty));
    EXPECT_TRUE(floats.empty());
}

TEST(VertexTest, StaticLayout)
{
    EXPECT_EQ(Vertex::LAYOUT.attributes.size(), 3);
    EXPECT_EQ(Vertex::LAYOUT.stride, static_cast<int32_t>(sizeof(float) * 8));
}
