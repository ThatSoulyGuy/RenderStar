#include "RenderStar/Client/Render/Shader/DefaultShader.hpp"
#include "RenderStar/Client/Render/Resource/Vertex.hpp"

namespace RenderStar::Client::Render
{
    Common::Utility::ResourcePath DefaultShader::GetVertexShaderPath() const
    {
        return Common::Utility::ResourcePath("renderstar", "shader/default.vert");
    }

    Common::Utility::ResourcePath DefaultShader::GetFragmentShaderPath() const
    {
        return Common::Utility::ResourcePath("renderstar", "shader/default.frag");
    }

    const VertexLayout& DefaultShader::GetRequiredVertexLayout() const
    {
        return Vertex::LAYOUT;
    }
}
