#include "RenderStar/Client/Render/Resource/Vertex.hpp"

namespace RenderStar::Client::Render
{
    const VertexLayout Vertex::LAYOUT = VertexLayout{
        {
            { 0, VertexAttributeType::FLOAT3, false, 0 },
            { 1, VertexAttributeType::FLOAT3, false, sizeof(float) * 3 },
            { 2, VertexAttributeType::FLOAT2, false, sizeof(float) * 6 }
        },
        static_cast<int32_t>(sizeof(float) * 8)
    };

    Vertex::Vertex()
        : posX(0.0f), posY(0.0f), posZ(0.0f)
        , colorR(1.0f), colorG(1.0f), colorB(1.0f)
        , texU(0.0f), texV(0.0f)
    {
    }

    Vertex::Vertex(float posX, float posY, float posZ,
                   float colorR, float colorG, float colorB,
                   float texU, float texV)
        : posX(posX), posY(posY), posZ(posZ)
        , colorR(colorR), colorG(colorG), colorB(colorB)
        , texU(texU), texV(texV)
    {
    }

    const VertexLayout& Vertex::GetLayout() const
    {
        return LAYOUT;
    }

    void Vertex::WriteTo(std::span<float> array, size_t offset) const
    {
        array[offset] = posX;
        array[offset + 1] = posY;
        array[offset + 2] = posZ;
        array[offset + 3] = colorR;
        array[offset + 4] = colorG;
        array[offset + 5] = colorB;
        array[offset + 6] = texU;
        array[offset + 7] = texV;
    }

    std::vector<float> Vertex::ToFloatArray(std::span<const Vertex> vertices)
    {
        if (vertices.empty())
            return {};

        constexpr int32_t floatsPerVertex = 8;
        std::vector<float> result(vertices.size() * floatsPerVertex);

        for (size_t i = 0; i < vertices.size(); ++i)
            vertices[i].WriteTo(result, i * floatsPerVertex);

        return result;
    }
}
