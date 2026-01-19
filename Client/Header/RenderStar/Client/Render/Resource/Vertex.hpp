#pragma once

#include "RenderStar/Client/Render/Resource/IVertex.hpp"
#include "RenderStar/Client/Render/Resource/VertexLayout.hpp"
#include <vector>
#include <span>

namespace RenderStar::Client::Render
{
    class Vertex final : public IVertex
    {
    public:

        static const VertexLayout LAYOUT;

        float posX, posY, posZ;
        float colorR, colorG, colorB;
        float texU, texV;

        Vertex();

        Vertex(float posX, float posY, float posZ,
               float colorR, float colorG, float colorB,
               float texU, float texV);

        const VertexLayout& GetLayout() const override;

        void WriteTo(std::span<float> array, size_t offset) const override;

        static std::vector<float> ToFloatArray(std::span<const Vertex> vertices);

        template<typename T>
        requires std::derived_from<T, IVertex>
        static std::vector<float> ToFloatArray(std::span<const T> vertices)
        {
            if (vertices.empty())
                return {};

            int32_t floatsPerVertex = static_cast<int32_t>(vertices[0].GetLayout().stride / sizeof(float));
            std::vector<float> result(vertices.size() * floatsPerVertex);

            for (size_t i = 0; i < vertices.size(); ++i)
                vertices[i].WriteTo(result, i * floatsPerVertex);

            return result;
        }
    };
}
