#pragma once

#include "RenderStar/Client/Render/Resource/VertexLayout.hpp"
#include <span>

namespace RenderStar::Client::Render
{
    class IVertex
    {
    public:

        virtual ~IVertex() = default;

        virtual const VertexLayout& GetLayout() const = 0;

        virtual void WriteTo(std::span<float> array, size_t offset) const = 0;
    };
}
