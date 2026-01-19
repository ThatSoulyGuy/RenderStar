#pragma once

#include <string>

namespace RenderStar::Client::Render::Shader
{
    enum class ShaderType
    {
        VERTEX,
        FRAGMENT,
        GEOMETRY,
        COMPUTE
    };

    class GlslTransformer
    {
    public:

        static std::string Transform450To410(const std::string& source, ShaderType shaderType);

        static bool CanTransform(const std::string& source);

    private:

        static std::string TransformVersion(const std::string& source);

        static std::string TransformUboBindings(const std::string& source);

        static std::string RemovePushConstants(const std::string& source);
    };
}
