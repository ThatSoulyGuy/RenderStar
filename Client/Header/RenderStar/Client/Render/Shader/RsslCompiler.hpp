#pragma once

#include "RenderStar/Client/Render/Shader/RsslTypes.hpp"
#include <string>

namespace RenderStar::Client::Render::Shader
{
    class RsslCompiler
    {
    public:

        static RsslParseResult Parse(const std::string& source);

        static RsslCompileResult Compile(const std::string& source);

        static RsslCompileResult Compile(const RsslParseResult& parsed);

    private:

        static std::string EmitStageGlsl(
            const RsslParseResult& parsed,
            const RsslStageSection& stage);

        static std::string EmitSharedDeclarations(const RsslParseResult& parsed);

        static std::string EmitStageInputs(const RsslStageSection& stage);

        static std::string EmitStageOutputs(const RsslStageSection& stage);
    };
}
