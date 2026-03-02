#include "RenderStar/Client/Render/Shader/GlslTransformer.hpp"
#include "RenderStar/Client/Render/Shader/GlslVersion.hpp"
#include <regex>
#include <spdlog/spdlog.h>

namespace RenderStar::Client::Render::Shader
{
    std::string GlslTransformer::Transform450To410(const std::string& source, ShaderType shaderType)
    {
        (void)shaderType;

        if (source.empty())
            return source;

        auto version = ParseVersionFromSource(source);

        if (!version.has_value())
            return source;

        if (version.value() != GlslVersion::GLSL_450 && version.value() != GlslVersion::GLSL_460)
        {
            spdlog::debug("Shader is not GLSL 450/460, skipping transformation");
            return source;
        }

        std::string result = source;

        result = TransformVersion(result);
        result = TransformUboBindings(result);
        result = RemovePushConstants(result);
        result = TransformBuiltins(result);
        result = TransformShadowCoordinates(result);

        spdlog::debug("Transformed shader from GLSL 450 to GLSL 410");

        return result;
    }

    bool GlslTransformer::CanTransform(const std::string& source)
    {
        auto version = ParseVersionFromSource(source);

        if (!version.has_value())
            return false;

        return version.value() == GlslVersion::GLSL_450 || version.value() == GlslVersion::GLSL_460;
    }

    std::string GlslTransformer::TransformVersion(const std::string& source)
    {
        std::regex versionPattern(R"(^\s*#version\s+(\d+)(\s+\w+)?\s*$)", std::regex::multiline);
        return std::regex_replace(source, versionPattern, "#version 450 core");
    }

    std::string GlslTransformer::TransformUboBindings(const std::string& source)
    {
        std::regex uboPattern(R"(layout\s*\(\s*binding\s*=\s*(\d+)\s*\)\s*(uniform\s+\w+\s*\{))");
        return std::regex_replace(source, uboPattern, "layout(binding = $1, std140) $2");
    }

    std::string GlslTransformer::RemovePushConstants(const std::string& source)
    {
        std::regex pushConstantPattern(R"(layout\s*\(\s*push_constant\s*\)\s*uniform[^;]*\{[^}]*\}\s*\w+\s*;)");
        return std::regex_replace(source, pushConstantPattern, "// Push constants removed for OpenGL compatibility");
    }

    std::string GlslTransformer::TransformBuiltins(const std::string& source)
    {
        std::string result = source;

        std::regex vertexIndexPattern(R"(\bgl_VertexIndex\b)");
        result = std::regex_replace(result, vertexIndexPattern, "gl_VertexID");

        std::regex instanceIndexPattern(R"(\bgl_InstanceIndex\b)");
        result = std::regex_replace(result, instanceIndexPattern, "gl_InstanceID");

        return result;
    }

    std::string GlslTransformer::TransformShadowCoordinates(const std::string& source)
    {
        std::regex yFlipPattern(R"(vec2\s*\(\s*0\.5\s*,\s*-0\.5\s*\))");
        return std::regex_replace(source, yFlipPattern, "vec2(0.5, 0.5)");
    }
}
