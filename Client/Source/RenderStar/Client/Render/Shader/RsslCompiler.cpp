#include "RenderStar/Client/Render/Shader/RsslCompiler.hpp"
#include <algorithm>
#include <regex>
#include <sstream>

namespace RenderStar::Client::Render::Shader
{
    bool RsslParseResult::HasStage(RsslStageType type) const
    {
        return GetStage(type) != nullptr;
    }

    const RsslStageSection* RsslParseResult::GetStage(RsslStageType type) const
    {
        for (const auto& stage : stages)
        {
            if (stage.type == type)
                return &stage;
        }

        return nullptr;
    }

    bool RsslParseResult::IsValid() const
    {
        return errors.empty() && version > 0 && !stages.empty();
    }

    bool RsslCompileResult::IsValid() const
    {
        return errors.empty();
    }

    static std::string Trim(const std::string& str)
    {
        auto start = str.find_first_not_of(" \t\r\n");

        if (start == std::string::npos)
            return "";

        auto end = str.find_last_not_of(" \t\r\n");

        return str.substr(start, end - start + 1);
    }

    static RsslStageType ParseStageType(const std::string& name)
    {
        if (name == "vertex")
            return RsslStageType::VERTEX;
        if (name == "fragment")
            return RsslStageType::FRAGMENT;
        if (name == "compute")
            return RsslStageType::COMPUTE;

        return RsslStageType::VERTEX;
    }

    RsslParseResult RsslCompiler::Parse(const std::string& source)
    {
        RsslParseResult result;
        std::istringstream stream(source);
        std::string line;
        bool foundVersion = false;
        bool inStage = false;
        bool inUniformBody = false;
        std::string currentUniformBody;
        std::string currentUniformName;
        int32_t currentUniformBinding = 0;
        int32_t braceDepth = 0;

        std::regex rsslVersionPattern(R"(^\s*#rssl\s+(\d+)\s*$)");
        std::regex stagePattern(R"(^\s*@stage\s+(\w+)\s*$)");
        std::regex uniformPattern(R"(^\s*@uniform\s+(\w+)\s*:\s*binding\s*\(\s*(\d+)\s*\)\s*$)");
        std::regex samplerPattern(R"(^\s*@(sampler\w*)\s+(\w+)\s*:\s*binding\s*\(\s*(\d+)\s*\)\s*$)");
        std::regex inputPattern(R"(^\s*@input\s+(\w+)\s*:\s*location\s*\(\s*(\d+)\s*\)\s*,\s*(\w+)\s*$)");
        std::regex outputPattern(R"(^\s*@output\s+(\w+)\s*:\s*location\s*\(\s*(\d+)\s*\)\s*,\s*(\w+)\s*$)");

        while (std::getline(stream, line))
        {
            std::smatch match;

            if (inUniformBody)
            {
                for (char c : line)
                {
                    if (c == '{')
                        braceDepth++;
                    else if (c == '}')
                        braceDepth--;
                }

                if (braceDepth <= 0)
                {
                    size_t closingBrace = line.find('}');

                    if (closingBrace != std::string::npos && closingBrace > 0)
                        currentUniformBody += line.substr(0, closingBrace) + "\n";

                    result.uniformBlocks.push_back({currentUniformName, currentUniformBinding, currentUniformBody});
                    inUniformBody = false;
                    currentUniformBody.clear();
                    continue;
                }

                currentUniformBody += line + "\n";
                continue;
            }

            if (!foundVersion)
            {
                std::string trimmed = Trim(line);

                if (trimmed.empty())
                    continue;

                if (std::regex_match(trimmed, match, rsslVersionPattern))
                {
                    result.version = std::stoi(match[1].str());
                    foundVersion = true;
                    continue;
                }

                result.errors.push_back("Expected #rssl version directive, got: " + trimmed);
                return result;
            }

            if (std::regex_match(line, match, stagePattern))
            {
                std::string typeName = match[1].str();

                if (typeName != "vertex" && typeName != "fragment" && typeName != "compute")
                {
                    result.errors.push_back("Unknown stage type: " + typeName);
                    continue;
                }

                inStage = true;
                RsslStageSection section;
                section.type = ParseStageType(typeName);
                result.stages.push_back(std::move(section));
                continue;
            }

            if (std::regex_match(line, match, uniformPattern))
            {
                currentUniformName = match[1].str();
                currentUniformBinding = std::stoi(match[2].str());
                inUniformBody = false;
                currentUniformBody.clear();
                braceDepth = 0;
                continue;
            }

            std::string trimmedLine = Trim(line);

            if (!currentUniformName.empty() && !inUniformBody && trimmedLine == "{")
            {
                inUniformBody = true;
                braceDepth = 1;
                continue;
            }

            if (std::regex_match(line, match, samplerPattern))
            {
                std::string samplerAnnotation = match[1].str();
                std::string samplerType = "sampler2D";

                if (samplerAnnotation == "sampler3D")
                    samplerType = "sampler3D";
                else if (samplerAnnotation == "samplerCube")
                    samplerType = "samplerCube";

                result.samplers.push_back({match[2].str(), std::stoi(match[3].str()), samplerType});
                continue;
            }

            if (inStage && std::regex_match(line, match, inputPattern))
            {
                RsslStageIO io;
                io.name = match[1].str();
                io.location = std::stoi(match[2].str());
                io.glslType = match[3].str();
                io.isInput = true;
                result.stages.back().inputs.push_back(std::move(io));
                continue;
            }

            if (inStage && std::regex_match(line, match, outputPattern))
            {
                RsslStageIO io;
                io.name = match[1].str();
                io.location = std::stoi(match[2].str());
                io.glslType = match[3].str();
                io.isInput = false;
                result.stages.back().outputs.push_back(std::move(io));
                continue;
            }

            if (inStage)
                result.stages.back().glslBody += line + "\n";
            else
                result.sharedGlsl += line + "\n";
        }

        if (inUniformBody)
            result.errors.push_back("Unterminated uniform block: " + currentUniformName);

        if (!foundVersion)
            result.errors.push_back("Missing #rssl version directive");

        if (result.version != 0 && result.version != 1)
            result.errors.push_back("Unsupported RSSL version: " + std::to_string(result.version));

        if (result.stages.empty() && result.errors.empty())
            result.errors.push_back("No @stage sections found");

        for (size_t i = 0; i < result.uniformBlocks.size(); i++)
        {
            for (size_t j = i + 1; j < result.uniformBlocks.size(); j++)
            {
                if (result.uniformBlocks[i].binding == result.uniformBlocks[j].binding)
                    result.errors.push_back("Duplicate binding(" + std::to_string(result.uniformBlocks[i].binding) + ") between '" + result.uniformBlocks[i].name + "' and '" + result.uniformBlocks[j].name + "'");
            }

            for (const auto& sampler : result.samplers)
            {
                if (result.uniformBlocks[i].binding == sampler.binding)
                    result.errors.push_back("Duplicate binding(" + std::to_string(sampler.binding) + ") between '" + result.uniformBlocks[i].name + "' and '" + sampler.name + "'");
            }
        }

        for (size_t i = 0; i < result.samplers.size(); i++)
        {
            for (size_t j = i + 1; j < result.samplers.size(); j++)
            {
                if (result.samplers[i].binding == result.samplers[j].binding)
                    result.errors.push_back("Duplicate binding(" + std::to_string(result.samplers[i].binding) + ") between '" + result.samplers[i].name + "' and '" + result.samplers[j].name + "'");
            }
        }

        for (const auto& stage : result.stages)
        {
            for (size_t i = 0; i < stage.inputs.size(); i++)
            {
                for (size_t j = i + 1; j < stage.inputs.size(); j++)
                {
                    if (stage.inputs[i].location == stage.inputs[j].location)
                        result.errors.push_back("Duplicate input location(" + std::to_string(stage.inputs[i].location) + ")");
                }
            }

            for (size_t i = 0; i < stage.outputs.size(); i++)
            {
                for (size_t j = i + 1; j < stage.outputs.size(); j++)
                {
                    if (stage.outputs[i].location == stage.outputs[j].location)
                        result.errors.push_back("Duplicate output location(" + std::to_string(stage.outputs[i].location) + ")");
                }
            }
        }

        {
            bool hasCompute = false;
            bool hasNonCompute = false;

            for (const auto& stage : result.stages)
            {
                if (stage.type == RsslStageType::COMPUTE)
                    hasCompute = true;
                else
                    hasNonCompute = true;
            }

            if (hasCompute && hasNonCompute)
                result.errors.push_back("Compute stages cannot be mixed with vertex/fragment stages");
        }

        for (size_t i = 0; i < result.stages.size(); i++)
        {
            for (size_t j = i + 1; j < result.stages.size(); j++)
            {
                if (result.stages[i].type == result.stages[j].type)
                    result.errors.push_back("Duplicate @stage type");
            }
        }

        return result;
    }

    RsslCompileResult RsslCompiler::Compile(const std::string& source)
    {
        auto parsed = Parse(source);
        return Compile(parsed);
    }

    RsslCompileResult RsslCompiler::Compile(const RsslParseResult& parsed)
    {
        RsslCompileResult result;

        if (!parsed.IsValid())
        {
            result.errors = parsed.errors;
            return result;
        }

        for (const auto& stage : parsed.stages)
        {
            std::string glsl = EmitStageGlsl(parsed, stage);

            switch (stage.type)
            {
            case RsslStageType::VERTEX:
                result.vertexGlsl = std::move(glsl);
                break;
            case RsslStageType::FRAGMENT:
                result.fragmentGlsl = std::move(glsl);
                break;
            case RsslStageType::COMPUTE:
                result.computeGlsl = std::move(glsl);
                break;
            }
        }

        return result;
    }

    std::string RsslCompiler::EmitStageGlsl(
        const RsslParseResult& parsed,
        const RsslStageSection& stage)
    {
        std::string output;

        output += "#version 450\n\n";
        output += EmitSharedDeclarations(parsed);

        std::string trimmedShared = Trim(parsed.sharedGlsl);

        if (!trimmedShared.empty())
            output += parsed.sharedGlsl + "\n";

        output += EmitStageInputs(stage);
        output += EmitStageOutputs(stage);

        if (!stage.inputs.empty() || !stage.outputs.empty())
            output += "\n";

        output += stage.glslBody;

        return output;
    }

    std::string RsslCompiler::EmitSharedDeclarations(const RsslParseResult& parsed)
    {
        std::string output;
        int32_t uboIndex = 0;

        for (const auto& block : parsed.uniformBlocks)
        {
            std::string instanceName;

            if (uboIndex == 0)
                instanceName = "ubo";
            else
            {
                instanceName = block.name;
                std::transform(instanceName.begin(), instanceName.end(), instanceName.begin(), ::tolower);
            }

            output += "layout(std140, binding = " + std::to_string(block.binding) + ") uniform " + block.name + " {\n";
            output += block.body;
            output += "} " + instanceName + ";\n\n";
            uboIndex++;
        }

        for (const auto& sampler : parsed.samplers)
        {
            output += "layout(binding = " + std::to_string(sampler.binding) + ") uniform " + sampler.samplerType + " " + sampler.name + ";\n";
        }

        if (!parsed.samplers.empty())
            output += "\n";

        return output;
    }

    std::string RsslCompiler::EmitStageInputs(const RsslStageSection& stage)
    {
        std::string output;

        for (const auto& input : stage.inputs)
        {
            output += "layout(location = " + std::to_string(input.location) + ") in " + input.glslType + " " + input.name + ";\n";
        }

        return output;
    }

    std::string RsslCompiler::EmitStageOutputs(const RsslStageSection& stage)
    {
        std::string output;

        for (const auto& out : stage.outputs)
        {
            output += "layout(location = " + std::to_string(out.location) + ") out " + out.glslType + " " + out.name + ";\n";
        }

        return output;
    }
}
