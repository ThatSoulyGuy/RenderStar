#pragma once

#include "RenderStar/Client/Render/Resource/IVertex.hpp"
#include "RenderStar/Client/Render/Resource/VertexLayout.hpp"
#include <span>

namespace RenderStar::Client::Render::Framework
{
    struct LitVertex final : public IVertex
    {
        static const VertexLayout LAYOUT;

        float posX, posY, posZ;
        float normalX, normalY, normalZ;
        float texU, texV;
        float tangentX, tangentY, tangentZ;

        LitVertex();

        LitVertex(float posX, float posY, float posZ,
                  float normalX, float normalY, float normalZ,
                  float texU, float texV);

        LitVertex(float posX, float posY, float posZ,
                  float normalX, float normalY, float normalZ,
                  float texU, float texV,
                  float tangentX, float tangentY, float tangentZ);

        const VertexLayout& GetLayout() const override;

        void WriteTo(std::span<float> array, size_t offset) const override;
    };
}
