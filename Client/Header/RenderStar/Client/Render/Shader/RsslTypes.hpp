#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace RenderStar::Client::Render::Shader
{
    enum class RsslStageType
    {
        VERTEX,
        FRAGMENT,
        COMPUTE
    };

    struct RsslUniformBlock
    {
        std::string name;
        int32_t binding;
        std::string body;
    };

    struct RsslSampler
    {
        std::string name;
        int32_t binding;
        std::string samplerType;
    };

    struct RsslStageIO
    {
        std::string name;
        int32_t location;
        std::string glslType;
        bool isInput;
    };

    struct RsslStageSection
    {
        RsslStageType type;
        std::vector<RsslStageIO> inputs;
        std::vector<RsslStageIO> outputs;
        std::string glslBody;
    };

    struct RsslParseResult
    {
        int32_t version = 0;
        std::vector<RsslUniformBlock> uniformBlocks;
        std::vector<RsslSampler> samplers;
        std::vector<RsslStageSection> stages;
        std::string sharedGlsl;
        std::vector<std::string> errors;

        bool HasStage(RsslStageType type) const;
        const RsslStageSection* GetStage(RsslStageType type) const;
        bool IsValid() const;
    };

    struct RsslCompileResult
    {
        std::string vertexGlsl;
        std::string fragmentGlsl;
        std::string computeGlsl;
        std::vector<std::string> errors;

        bool IsValid() const;
    };
}
