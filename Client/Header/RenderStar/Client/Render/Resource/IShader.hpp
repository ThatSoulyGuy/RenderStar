#pragma once

#include "RenderStar/Common/Utility/ResourcePath.hpp"

namespace RenderStar::Client::Render
{
    struct VertexLayout;

    class IShader
    {
    public:

        virtual ~IShader() = default;

        virtual Common::Utility::ResourcePath GetVertexShaderPath() const = 0;
        virtual Common::Utility::ResourcePath GetFragmentShaderPath() const = 0;
        virtual const VertexLayout& GetRequiredVertexLayout() const = 0;
    };
}
