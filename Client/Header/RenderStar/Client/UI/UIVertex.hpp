#pragma once

#include "RenderStar/Client/Render/Resource/IVertex.hpp"
#include "RenderStar/Client/Render/Resource/VertexLayout.hpp"
#include <span>

namespace RenderStar::Client::UI
{
    struct UIVertex final : public Render::IVertex
    {
        static const Render::VertexLayout LAYOUT;

        float posX, posY;
        float texU, texV;

        UIVertex();
        UIVertex(float posX, float posY, float texU, float texV);

        const Render::VertexLayout& GetLayout() const override;
        void WriteTo(std::span<float> array, size_t offset) const override;
    };
}
