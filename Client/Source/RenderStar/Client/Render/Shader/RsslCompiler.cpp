#include "RenderStar/Client/Render/Shader/RsslCompiler.hpp"
#include <algorithm>
#include <regex>
#include <set>
#include <sstream>

namespace RenderStar::Client::Render::Shader
{
    // ── Public result type methods (unchanged API) ──────────────────────

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

    // ── Diagnostic formatting ──────────────────────────────────────────

    std::string RsslDiagnostic::Format() const
    {
        std::string result;

        if (!location.file.empty())
            result += location.file + ":";

        if (location.line > 0)
        {
            result += std::to_string(location.line);

            if (location.column > 0)
                result += ":" + std::to_string(location.column);

            result += ": ";
        }

        result += (severity == Severity::SEV_ERROR ? "error: " : "warning: ");
        result += message;

        return result;
    }

    // ── Internal implementation ─────────────────────────────────────────

    namespace
    {
        // ── Helpers ────────────────────────────────────────────────────

        std::string Trim(const std::string& str)
        {
            auto start = str.find_first_not_of(" \t\r\n");

            if (start == std::string::npos)
                return "";

            auto end = str.find_last_not_of(" \t\r\n");

            return str.substr(start, end - start + 1);
        }

        RsslStageType ParseStageType(const std::string& name)
        {
            if (name == "vertex")
                return RsslStageType::VERTEX;
            if (name == "fragment")
                return RsslStageType::FRAGMENT;
            if (name == "compute")
                return RsslStageType::COMPUTE;

            return RsslStageType::VERTEX;
        }

        // ── Compiled regex patterns ────────────────────────────────────

        const std::regex& VersionPattern()
        {
            static const std::regex p(R"(^\s*#rssl\s+(\d+)\s*$)");
            return p;
        }

        const std::regex& StagePattern()
        {
            static const std::regex p(R"(^\s*@stage\s+(\w+)\s*$)");
            return p;
        }

        const std::regex& UniformPattern()
        {
            static const std::regex p(R"(^\s*@uniform\s+(\w+)\s*:\s*binding\s*\(\s*(\d+)\s*\)\s*$)");
            return p;
        }

        const std::regex& SamplerPattern()
        {
            static const std::regex p(R"(^\s*@(sampler\w*)\s+(\w+)\s*:\s*binding\s*\(\s*(\d+)\s*\)\s*$)");
            return p;
        }

        const std::regex& InputPattern()
        {
            static const std::regex p(R"(^\s*@input\s+(\w+)\s*:\s*location\s*\(\s*(\d+)\s*\)\s*,\s*(\w+)\s*$)");
            return p;
        }

        const std::regex& OutputPattern()
        {
            static const std::regex p(R"(^\s*@output\s+(\w+)\s*:\s*location\s*\(\s*(\d+)\s*\)\s*,\s*(\w+)\s*$)");
            return p;
        }

        const std::regex& IncludePattern()
        {
            static const std::regex p("^\\s*#include\\s+\"([^\"]+)\"\\s*$");
            return p;
        }

        const std::regex& DefinePattern()
        {
            static const std::regex p(R"(^\s*#define\s+(\w+)\s+(.*?)\s*$)");
            return p;
        }

        // ── Line scanner → AST builder ─────────────────────────────────

        std::unique_ptr<RsslProgram> ScanAndBuildAst(
            const std::string& source,
            const std::string& fileName,
            const IncludeResolver* resolver,
            std::vector<RsslDiagnostic>& diagnostics,
            bool requireVersion = true)
        {
            auto program = std::make_unique<RsslProgram>();
            program->location = {fileName, 1, 1};

            std::istringstream stream(source);
            std::string line;
            int32_t lineNum = 0;

            bool foundVersion = !requireVersion;
            bool inUniformBody = false;
            std::string currentUniformBody;
            std::string currentUniformName;
            int32_t currentUniformBinding = 0;
            int32_t currentUniformLine = 0;
            int32_t braceDepth = 0;

            RsslStageSectionNode* currentStage = nullptr;
            RsslSharedGlsl* currentSharedGlsl = nullptr;

            auto addError = [&](int32_t ln, const std::string& msg)
            {
                diagnostics.push_back({RsslDiagnostic::Severity::SEV_ERROR, msg, {fileName, ln, 1}});
            };

            while (std::getline(stream, line))
            {
                lineNum++;
                std::smatch match;

                // ── Uniform body accumulation ──

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

                        auto node = std::make_unique<RsslUniformBlockNode>();
                        node->location = {fileName, currentUniformLine, 1};
                        node->name = std::move(currentUniformName);
                        node->binding = currentUniformBinding;
                        node->rawBody = std::move(currentUniformBody);
                        program->declarations.push_back(std::move(node));

                        inUniformBody = false;
                        currentUniformBody.clear();
                        currentUniformName.clear();
                        continue;
                    }

                    currentUniformBody += line + "\n";
                    continue;
                }

                // ── Version directive (must be first non-empty line) ──

                if (!foundVersion)
                {
                    std::string trimmed = Trim(line);

                    if (trimmed.empty())
                        continue;

                    if (std::regex_match(trimmed, match, VersionPattern()))
                    {
                        program->version = std::stoi(match[1].str());

                        auto node = std::make_unique<RsslVersionDirective>();
                        node->location = {fileName, lineNum, 1};
                        node->version = program->version;
                        program->declarations.push_back(std::move(node));

                        foundVersion = true;
                        continue;
                    }

                    addError(lineNum, "Expected #rssl version directive, got: " + trimmed);
                    return program;
                }

                // ── @stage ──

                if (std::regex_match(line, match, StagePattern()))
                {
                    std::string typeName = match[1].str();

                    if (typeName != "vertex" && typeName != "fragment" && typeName != "compute")
                    {
                        addError(lineNum, "Unknown stage type: " + typeName);
                        continue;
                    }

                    auto node = std::make_unique<RsslStageSectionNode>();
                    node->location = {fileName, lineNum, 1};
                    node->stageType = ParseStageType(typeName);
                    currentStage = node.get();
                    currentSharedGlsl = nullptr;
                    program->declarations.push_back(std::move(node));
                    continue;
                }

                // ── @uniform header ──

                if (std::regex_match(line, match, UniformPattern()))
                {
                    currentUniformName = match[1].str();
                    currentUniformBinding = std::stoi(match[2].str());
                    currentUniformLine = lineNum;
                    inUniformBody = false;
                    currentUniformBody.clear();
                    braceDepth = 0;
                    continue;
                }

                // ── { after uniform header ──

                std::string trimmedLine = Trim(line);

                if (!currentUniformName.empty() && !inUniformBody && trimmedLine == "{")
                {
                    inUniformBody = true;
                    braceDepth = 1;
                    continue;
                }

                // ── @sampler ──

                if (std::regex_match(line, match, SamplerPattern()))
                {
                    std::string annotation = match[1].str();
                    std::string samplerType = "sampler2D";

                    if (annotation == "sampler3D")
                        samplerType = "sampler3D";
                    else if (annotation == "samplerCube")
                        samplerType = "samplerCube";

                    auto node = std::make_unique<RsslSamplerDeclNode>();
                    node->location = {fileName, lineNum, 1};
                    node->name = match[2].str();
                    node->binding = std::stoi(match[3].str());
                    node->samplerType = samplerType;
                    program->declarations.push_back(std::move(node));
                    continue;
                }

                // ── @input (inside stage only) ──

                if (currentStage && std::regex_match(line, match, InputPattern()))
                {
                    RsslStageIODecl io;
                    io.name = match[1].str();
                    io.location = std::stoi(match[2].str());
                    io.glslType = match[3].str();
                    io.isInput = true;
                    currentStage->inputs.push_back(std::move(io));
                    continue;
                }

                // ── @output (inside stage only) ──

                if (currentStage && std::regex_match(line, match, OutputPattern()))
                {
                    RsslStageIODecl io;
                    io.name = match[1].str();
                    io.location = std::stoi(match[2].str());
                    io.glslType = match[3].str();
                    io.isInput = false;
                    currentStage->outputs.push_back(std::move(io));
                    continue;
                }

                // ── #include (new feature, requires resolver) ──

                if (resolver && std::regex_match(line, match, IncludePattern()))
                {
                    auto node = std::make_unique<RsslIncludeDirective>();
                    node->location = {fileName, lineNum, 1};
                    node->path = match[1].str();
                    currentSharedGlsl = nullptr;
                    program->declarations.push_back(std::move(node));
                    continue;
                }

                // ── #define (new feature, requires resolver) ──

                if (resolver && std::regex_match(line, match, DefinePattern()))
                {
                    auto node = std::make_unique<RsslDefineDirective>();
                    node->location = {fileName, lineNum, 1};
                    node->name = match[1].str();
                    node->value = match[2].str();
                    program->declarations.push_back(std::move(node));
                    continue;
                }

                // ── Default: GLSL text ──

                if (currentStage)
                {
                    currentStage->glslBody += line + "\n";
                }
                else
                {
                    if (!currentSharedGlsl)
                    {
                        auto node = std::make_unique<RsslSharedGlsl>();
                        node->location = {fileName, lineNum, 1};
                        currentSharedGlsl = node.get();
                        program->declarations.push_back(std::move(node));
                    }

                    currentSharedGlsl->code += line + "\n";
                }
            }

            if (inUniformBody)
                addError(lineNum, "Unterminated uniform block: " + currentUniformName);

            if (!foundVersion)
                addError(lineNum, "Missing #rssl version directive");

            return program;
        }

        // ── Semantic validation ────────────────────────────────────────

        void ValidateProgram(
            const RsslProgram& program,
            std::vector<RsslDiagnostic>& diagnostics)
        {
            auto addError = [&](const RsslSourceLocation& loc, const std::string& msg)
            {
                diagnostics.push_back({RsslDiagnostic::Severity::SEV_ERROR, msg, loc});
            };

            if (program.version != 0 && program.version != 1)
                addError(program.location, "Unsupported RSSL version: " + std::to_string(program.version));

            std::vector<const RsslUniformBlockNode*> uniformBlocks;
            std::vector<const RsslSamplerDeclNode*> samplers;
            std::vector<const RsslStageSectionNode*> stages;

            for (const auto& decl : program.declarations)
            {
                switch (decl->kind)
                {
                case RsslNodeKind::UNIFORM_BLOCK:
                    uniformBlocks.push_back(static_cast<const RsslUniformBlockNode*>(decl.get()));
                    break;
                case RsslNodeKind::SAMPLER_DECL:
                    samplers.push_back(static_cast<const RsslSamplerDeclNode*>(decl.get()));
                    break;
                case RsslNodeKind::STAGE_SECTION:
                    stages.push_back(static_cast<const RsslStageSectionNode*>(decl.get()));
                    break;
                default:
                    break;
                }
            }

            if (stages.empty() && diagnostics.empty())
                addError(program.location, "No @stage sections found");

            // Duplicate bindings among uniform blocks
            for (size_t i = 0; i < uniformBlocks.size(); i++)
            {
                for (size_t j = i + 1; j < uniformBlocks.size(); j++)
                {
                    if (uniformBlocks[i]->binding == uniformBlocks[j]->binding)
                    {
                        addError(uniformBlocks[j]->location,
                            "Duplicate binding(" + std::to_string(uniformBlocks[i]->binding)
                            + ") between '" + uniformBlocks[i]->name
                            + "' and '" + uniformBlocks[j]->name + "'");
                    }
                }

                for (const auto* sampler : samplers)
                {
                    if (uniformBlocks[i]->binding == sampler->binding)
                    {
                        addError(sampler->location,
                            "Duplicate binding(" + std::to_string(sampler->binding)
                            + ") between '" + uniformBlocks[i]->name
                            + "' and '" + sampler->name + "'");
                    }
                }
            }

            // Duplicate bindings among samplers
            for (size_t i = 0; i < samplers.size(); i++)
            {
                for (size_t j = i + 1; j < samplers.size(); j++)
                {
                    if (samplers[i]->binding == samplers[j]->binding)
                    {
                        addError(samplers[j]->location,
                            "Duplicate binding(" + std::to_string(samplers[i]->binding)
                            + ") between '" + samplers[i]->name
                            + "' and '" + samplers[j]->name + "'");
                    }
                }
            }

            // Per-stage validation
            for (const auto* stage : stages)
            {
                for (size_t i = 0; i < stage->inputs.size(); i++)
                {
                    for (size_t j = i + 1; j < stage->inputs.size(); j++)
                    {
                        if (stage->inputs[i].location == stage->inputs[j].location)
                            addError(stage->location,
                                "Duplicate input location(" + std::to_string(stage->inputs[i].location) + ")");
                    }
                }

                for (size_t i = 0; i < stage->outputs.size(); i++)
                {
                    for (size_t j = i + 1; j < stage->outputs.size(); j++)
                    {
                        if (stage->outputs[i].location == stage->outputs[j].location)
                            addError(stage->location,
                                "Duplicate output location(" + std::to_string(stage->outputs[i].location) + ")");
                    }
                }
            }

            // Compute + non-compute mixing
            {
                bool hasCompute = false;
                bool hasNonCompute = false;

                for (const auto* stage : stages)
                {
                    if (stage->stageType == RsslStageType::COMPUTE)
                        hasCompute = true;
                    else
                        hasNonCompute = true;
                }

                if (hasCompute && hasNonCompute)
                    addError(program.location, "Compute stages cannot be mixed with vertex/fragment stages");
            }

            // Duplicate stage types
            for (size_t i = 0; i < stages.size(); i++)
            {
                for (size_t j = i + 1; j < stages.size(); j++)
                {
                    if (stages[i]->stageType == stages[j]->stageType)
                        addError(stages[j]->location, "Duplicate @stage type");
                }
            }
        }

        // ── Include resolution ─────────────────────────────────────────

        void ResolveIncludes(
            RsslProgram& program,
            const IncludeResolver& resolver,
            const std::string& currentFile,
            std::vector<RsslDiagnostic>& diagnostics,
            std::set<std::string>& resolvedFiles)
        {
            for (size_t i = 0; i < program.declarations.size(); i++)
            {
                if (program.declarations[i]->kind != RsslNodeKind::INCLUDE_DIRECTIVE)
                    continue;

                auto* include = static_cast<RsslIncludeDirective*>(program.declarations[i].get());

                if (resolvedFiles.count(include->path))
                    continue;

                std::string resolvedSource;

                try
                {
                    resolvedSource = resolver(include->path, currentFile);
                }
                catch (const std::exception& e)
                {
                    diagnostics.push_back({
                        RsslDiagnostic::Severity::SEV_ERROR,
                        "Failed to resolve #include \"" + include->path + "\": " + e.what(),
                        include->location});
                    continue;
                }

                resolvedFiles.insert(include->path);

                auto subProgram = ScanAndBuildAst(resolvedSource, include->path, &resolver, diagnostics, false);
                ResolveIncludes(*subProgram, resolver, include->path, diagnostics, resolvedFiles);

                include->resolvedNodes = std::move(subProgram->declarations);
            }
        }

        // ── Define application ─────────────────────────────────────────

        void ApplyDefines(RsslProgram& program)
        {
            std::vector<std::pair<std::string, std::string>> defines;

            for (const auto& decl : program.declarations)
            {
                if (decl->kind == RsslNodeKind::DEFINE_DIRECTIVE)
                {
                    auto* def = static_cast<const RsslDefineDirective*>(decl.get());

                    bool replaced = false;

                    for (auto& [name, value] : defines)
                    {
                        if (name == def->name)
                        {
                            value = def->value;
                            replaced = true;
                            break;
                        }
                    }

                    if (!replaced)
                        defines.emplace_back(def->name, def->value);
                }
            }

            if (defines.empty())
                return;

            auto substitute = [&](std::string& text)
            {
                for (const auto& [name, value] : defines)
                {
                    std::regex pattern("\\b" + name + "\\b");
                    text = std::regex_replace(text, pattern, value);
                }
            };

            for (auto& decl : program.declarations)
            {
                if (decl->kind == RsslNodeKind::STAGE_SECTION)
                {
                    auto* stage = static_cast<RsslStageSectionNode*>(decl.get());
                    substitute(stage->glslBody);
                }
                else if (decl->kind == RsslNodeKind::UNIFORM_BLOCK)
                {
                    auto* ubo = static_cast<RsslUniformBlockNode*>(decl.get());
                    substitute(ubo->rawBody);
                }
                else if (decl->kind == RsslNodeKind::SHARED_GLSL)
                {
                    auto* shared = static_cast<RsslSharedGlsl*>(decl.get());
                    substitute(shared->code);
                }
            }
        }

        // ── AST → RsslParseResult conversion ──────────────────────────

        void CollectFromDeclarations(
            const std::vector<std::unique_ptr<RsslAstNode>>& declarations,
            RsslParseResult& result)
        {
            for (const auto& decl : declarations)
            {
                switch (decl->kind)
                {
                case RsslNodeKind::UNIFORM_BLOCK:
                {
                    auto* node = static_cast<const RsslUniformBlockNode*>(decl.get());
                    result.uniformBlocks.push_back({node->name, node->binding, node->rawBody});
                    break;
                }
                case RsslNodeKind::SAMPLER_DECL:
                {
                    auto* node = static_cast<const RsslSamplerDeclNode*>(decl.get());
                    result.samplers.push_back({node->name, node->binding, node->samplerType});
                    break;
                }
                case RsslNodeKind::STAGE_SECTION:
                {
                    auto* node = static_cast<const RsslStageSectionNode*>(decl.get());
                    RsslStageSection section;
                    section.type = node->stageType;
                    section.glslBody = node->glslBody;

                    for (const auto& io : node->inputs)
                        section.inputs.push_back({io.name, io.location, io.glslType, io.isInput});

                    for (const auto& io : node->outputs)
                        section.outputs.push_back({io.name, io.location, io.glslType, io.isInput});

                    result.stages.push_back(std::move(section));
                    break;
                }
                case RsslNodeKind::SHARED_GLSL:
                {
                    auto* node = static_cast<const RsslSharedGlsl*>(decl.get());
                    result.sharedGlsl += node->code;
                    break;
                }
                case RsslNodeKind::INCLUDE_DIRECTIVE:
                {
                    auto* node = static_cast<const RsslIncludeDirective*>(decl.get());
                    CollectFromDeclarations(node->resolvedNodes, result);
                    break;
                }
                default:
                    break;
                }
            }
        }

        RsslParseResult BuildParseResult(
            const RsslProgram& program,
            const std::vector<RsslDiagnostic>& diagnostics)
        {
            RsslParseResult result;
            result.version = program.version;

            CollectFromDeclarations(program.declarations, result);

            for (const auto& diag : diagnostics)
            {
                if (diag.severity == RsslDiagnostic::Severity::SEV_ERROR)
                    result.errors.push_back(diag.Format());
            }

            return result;
        }

        // ── Target-specific post-processing ──────────────────────────

        std::string PostProcessForTarget(std::string glsl, RsslTarget target)
        {
            if (target == RsslTarget::OPENGL_GLSL)
            {
                std::regex vertexIndexPattern(R"(\bgl_VertexIndex\b)");
                glsl = std::regex_replace(glsl, vertexIndexPattern, "gl_VertexID");

                std::regex instanceIndexPattern(R"(\bgl_InstanceIndex\b)");
                glsl = std::regex_replace(glsl, instanceIndexPattern, "gl_InstanceID");

                std::regex yFlipPattern(R"(vec2\s*\(\s*0\.5\s*,\s*-0\.5\s*\))");
                glsl = std::regex_replace(glsl, yFlipPattern, "vec2(0.5, 0.5)");
            }

            return glsl;
        }

        std::string GetVersionDirective(RsslTarget target)
        {
            switch (target)
            {
            case RsslTarget::OPENGL_GLSL:
                return "#version 410 core\n\n";
            case RsslTarget::VULKAN_GLSL:
            default:
                return "#version 450\n\n";
            }
        }

    } // anonymous namespace

    // ── GLSL emission ───────────────────────────────────────────────────

    std::string RsslCompiler::EmitStageGlsl(
        const RsslParseResult& parsed,
        const RsslStageSection& stage,
        RsslTarget target)
    {
        std::string output;

        output += GetVersionDirective(target);
        output += EmitSharedDeclarations(parsed);

        std::string trimmedShared = Trim(parsed.sharedGlsl);

        if (!trimmedShared.empty())
            output += parsed.sharedGlsl + "\n";

        output += EmitStageInputs(stage);
        output += EmitStageOutputs(stage);

        if (!stage.inputs.empty() || !stage.outputs.empty())
            output += "\n";

        output += stage.glslBody;

        return PostProcessForTarget(std::move(output), target);
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

    // ── Public API ──────────────────────────────────────────────────────

    RsslParseResult RsslCompiler::Parse(const std::string& source)
    {
        std::vector<RsslDiagnostic> diagnostics;
        auto program = ScanAndBuildAst(source, "<source>", nullptr, diagnostics);
        ValidateProgram(*program, diagnostics);

        return BuildParseResult(*program, diagnostics);
    }

    RsslParseResult RsslCompiler::Parse(
        const std::string& source,
        const IncludeResolver& resolver,
        const std::string& fileName)
    {
        std::vector<RsslDiagnostic> diagnostics;
        auto program = ScanAndBuildAst(source, fileName, &resolver, diagnostics);

        std::set<std::string> resolvedFiles;
        resolvedFiles.insert(fileName);
        ResolveIncludes(*program, resolver, fileName, diagnostics, resolvedFiles);

        ApplyDefines(*program);
        ValidateProgram(*program, diagnostics);

        return BuildParseResult(*program, diagnostics);
    }

    RsslCompileResult RsslCompiler::Compile(const std::string& source, RsslTarget target)
    {
        auto parsed = Parse(source);
        return Compile(parsed, target);
    }

    RsslCompileResult RsslCompiler::Compile(
        const std::string& source,
        const IncludeResolver& resolver,
        const std::string& fileName,
        RsslTarget target)
    {
        auto parsed = Parse(source, resolver, fileName);
        return Compile(parsed, target);
    }

    RsslCompileResult RsslCompiler::Compile(const RsslParseResult& parsed, RsslTarget target)
    {
        RsslCompileResult result;

        if (!parsed.IsValid())
        {
            result.errors = parsed.errors;
            return result;
        }

        for (const auto& stage : parsed.stages)
        {
            std::string glsl = EmitStageGlsl(parsed, stage, target);

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
}
