#pragma once

#include "RenderStar/Client/Render/Resource/IShader.hpp"
#include "RenderStar/Client/Render/Resource/VertexLayout.hpp"
#include "RenderStar/Common/Utility/ResourcePath.hpp"

namespace RenderStar::Client::Render
{
    class DefaultShader final : public IShader
    {
    public:

        Common::Utility::ResourcePath GetVertexShaderPath() const override;
        Common::Utility::ResourcePath GetFragmentShaderPath() const override;
        const VertexLayout& GetRequiredVertexLayout() const override;
    };
}
