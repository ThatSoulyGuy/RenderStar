#include "RenderStar/Client/Render/Framework/LitVertex.hpp"

namespace RenderStar::Client::Render::Framework
{
    const VertexLayout LitVertex::LAYOUT = VertexLayout{
        {
            { 0, VertexAttributeType::FLOAT3, false, 0 },
            { 1, VertexAttributeType::FLOAT3, false, sizeof(float) * 3 },
            { 2, VertexAttributeType::FLOAT2, false, sizeof(float) * 6 },
            { 3, VertexAttributeType::FLOAT3, false, sizeof(float) * 8 }
        },
        static_cast<int32_t>(sizeof(float) * 11)
    };

    LitVertex::LitVertex()
        : posX(0.0f), posY(0.0f), posZ(0.0f)
        , normalX(0.0f), normalY(1.0f), normalZ(0.0f)
        , texU(0.0f), texV(0.0f)
        , tangentX(1.0f), tangentY(0.0f), tangentZ(0.0f)
    {
    }

    LitVertex::LitVertex(float posX, float posY, float posZ,
                         float normalX, float normalY, float normalZ,
                         float texU, float texV)
        : posX(posX), posY(posY), posZ(posZ)
        , normalX(normalX), normalY(normalY), normalZ(normalZ)
        , texU(texU), texV(texV)
        , tangentX(1.0f), tangentY(0.0f), tangentZ(0.0f)
    {
    }

    LitVertex::LitVertex(float posX, float posY, float posZ,
                         float normalX, float normalY, float normalZ,
                         float texU, float texV,
                         float tangentX, float tangentY, float tangentZ)
        : posX(posX), posY(posY), posZ(posZ)
        , normalX(normalX), normalY(normalY), normalZ(normalZ)
        , texU(texU), texV(texV)
        , tangentX(tangentX), tangentY(tangentY), tangentZ(tangentZ)
    {
    }

    const VertexLayout& LitVertex::GetLayout() const
    {
        return LAYOUT;
    }

    void LitVertex::WriteTo(std::span<float> array, size_t offset) const
    {
        array[offset] = posX;
        array[offset + 1] = posY;
        array[offset + 2] = posZ;
        array[offset + 3] = normalX;
        array[offset + 4] = normalY;
        array[offset + 5] = normalZ;
        array[offset + 6] = texU;
        array[offset + 7] = texV;
        array[offset + 8] = tangentX;
        array[offset + 9] = tangentY;
        array[offset + 10] = tangentZ;
    }
}
