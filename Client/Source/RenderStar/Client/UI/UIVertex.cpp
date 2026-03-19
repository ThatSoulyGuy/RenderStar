#include "RenderStar/Client/UI/UIVertex.hpp"

namespace RenderStar::Client::UI
{
    const Render::VertexLayout UIVertex::LAYOUT = Render::VertexLayout{
        {
            { 0, Render::VertexAttributeType::FLOAT2, false, 0 },
            { 1, Render::VertexAttributeType::FLOAT2, false, sizeof(float) * 2 }
        },
        static_cast<int32_t>(sizeof(float) * 4)
    };

    UIVertex::UIVertex()
        : posX(0.0f), posY(0.0f), texU(0.0f), texV(0.0f)
    {
    }

    UIVertex::UIVertex(float posX, float posY, float texU, float texV)
        : posX(posX), posY(posY), texU(texU), texV(texV)
    {
    }

    const Render::VertexLayout& UIVertex::GetLayout() const
    {
        return LAYOUT;
    }

    void UIVertex::WriteTo(std::span<float> array, size_t offset) const
    {
        array[offset]     = posX;
        array[offset + 1] = posY;
        array[offset + 2] = texU;
        array[offset + 3] = texV;
    }
}
