#pragma once

#include "RenderStar/Client/Render/Shader/RsslTypes.hpp"
#include <string>

namespace RenderStar::Client::Render::Shader
{
    class RsslCompiler
    {
    public:

        static RsslParseResult Parse(const std::string& source);

        static RsslParseResult Parse(
            const std::string& source,
            const IncludeResolver& resolver,
            const std::string& fileName = "<source>");

        static RsslCompileResult Compile(
            const std::string& source,
            RsslTarget target = RsslTarget::VULKAN_GLSL);

        static RsslCompileResult Compile(
            const std::string& source,
            const IncludeResolver& resolver,
            const std::string& fileName = "<source>",
            RsslTarget target = RsslTarget::VULKAN_GLSL);

        static RsslCompileResult Compile(
            const RsslParseResult& parsed,
            RsslTarget target = RsslTarget::VULKAN_GLSL);

    private:

        static std::string EmitStageGlsl(
            const RsslParseResult& parsed,
            const RsslStageSection& stage,
            RsslTarget target);

        static std::string EmitSharedDeclarations(const RsslParseResult& parsed);

        static std::string EmitStageInputs(const RsslStageSection& stage);

        static std::string EmitStageOutputs(const RsslStageSection& stage);
    };
}
