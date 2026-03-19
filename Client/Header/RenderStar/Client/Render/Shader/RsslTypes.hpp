#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace RenderStar::Client::Render::Shader
{
    // ── Existing types (public API, unchanged) ──────────────────────────

    enum class RsslStageType
    {
        VERTEX,
        FRAGMENT,
        COMPUTE
    };

    enum class RsslTarget
    {
        VULKAN_GLSL,
        OPENGL_GLSL
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

    // ── Token types ─────────────────────────────────────────────────────

    enum class RsslTokenType
    {
        RSSL_DIRECTIVE,
        INCLUDE_DIRECTIVE,
        DEFINE_DIRECTIVE,

        AT_STAGE,
        AT_UNIFORM,
        AT_SAMPLER,
        AT_INPUT,
        AT_OUTPUT,

        KW_STRUCT,
        KW_CONST,

        IDENTIFIER,
        TYPE_NAME,

        INTEGER_LITERAL,
        FLOAT_LITERAL,
        STRING_LITERAL,

        LBRACE,
        RBRACE,
        LPAREN,
        RPAREN,
        LBRACKET,
        RBRACKET,
        COLON,
        SEMICOLON,
        COMMA,
        EQUALS,
        DOT,

        GLSL_LINE,
        NEWLINE,
        END_OF_FILE,
        TOKEN_ERROR
    };

    // ── Source location ─────────────────────────────────────────────────

    struct RsslSourceLocation
    {
        std::string file;
        int32_t line = 0;
        int32_t column = 0;
    };

    // ── Token ───────────────────────────────────────────────────────────

    struct RsslToken
    {
        RsslTokenType type = RsslTokenType::TOKEN_ERROR;
        std::string value;
        RsslSourceLocation location;
    };

    // ── Diagnostic ──────────────────────────────────────────────────────

    struct RsslDiagnostic
    {
        enum class Severity
        {
            SEV_ERROR,
            SEV_WARNING
        };

        Severity severity = Severity::SEV_ERROR;
        std::string message;
        RsslSourceLocation location;

        std::string Format() const;
    };

    // ── AST node types ──────────────────────────────────────────────────

    enum class RsslNodeKind
    {
        VERSION_DIRECTIVE,
        INCLUDE_DIRECTIVE,
        DEFINE_DIRECTIVE,
        STRUCT_DECL,
        UNIFORM_BLOCK,
        SAMPLER_DECL,
        CONSTANT_DECL,
        STAGE_SECTION,
        SHARED_GLSL,
        PROGRAM
    };

    struct RsslAstNode
    {
        RsslNodeKind kind;
        RsslSourceLocation location;

        explicit RsslAstNode(RsslNodeKind k) : kind(k) {}
        virtual ~RsslAstNode() = default;

        RsslAstNode(const RsslAstNode&) = delete;
        RsslAstNode& operator=(const RsslAstNode&) = delete;
        RsslAstNode(RsslAstNode&&) = default;
        RsslAstNode& operator=(RsslAstNode&&) = default;
    };

    struct RsslVersionDirective : RsslAstNode
    {
        int32_t version = 0;

        RsslVersionDirective() : RsslAstNode(RsslNodeKind::VERSION_DIRECTIVE) {}
    };

    struct RsslIncludeDirective : RsslAstNode
    {
        std::string path;
        std::vector<std::unique_ptr<RsslAstNode>> resolvedNodes;

        RsslIncludeDirective() : RsslAstNode(RsslNodeKind::INCLUDE_DIRECTIVE) {}
    };

    struct RsslDefineDirective : RsslAstNode
    {
        std::string name;
        std::string value;

        RsslDefineDirective() : RsslAstNode(RsslNodeKind::DEFINE_DIRECTIVE) {}
    };

    struct RsslStructField
    {
        std::string type;
        std::string name;
        int32_t arraySize = 0;
    };

    struct RsslStructDecl : RsslAstNode
    {
        std::string name;
        std::vector<RsslStructField> fields;

        RsslStructDecl() : RsslAstNode(RsslNodeKind::STRUCT_DECL) {}
    };

    struct RsslUniformBlockNode : RsslAstNode
    {
        std::string name;
        int32_t binding = 0;
        std::string rawBody;

        RsslUniformBlockNode() : RsslAstNode(RsslNodeKind::UNIFORM_BLOCK) {}
    };

    struct RsslSamplerDeclNode : RsslAstNode
    {
        std::string name;
        int32_t binding = 0;
        std::string samplerType;

        RsslSamplerDeclNode() : RsslAstNode(RsslNodeKind::SAMPLER_DECL) {}
    };

    struct RsslConstantDecl : RsslAstNode
    {
        std::string type;
        std::string name;
        std::string value;

        RsslConstantDecl() : RsslAstNode(RsslNodeKind::CONSTANT_DECL) {}
    };

    struct RsslStageIODecl
    {
        std::string name;
        int32_t location = 0;
        std::string glslType;
        bool isInput = true;
    };

    struct RsslStageSectionNode : RsslAstNode
    {
        RsslStageType stageType = RsslStageType::VERTEX;
        std::vector<RsslStageIODecl> inputs;
        std::vector<RsslStageIODecl> outputs;
        std::string glslBody;

        RsslStageSectionNode() : RsslAstNode(RsslNodeKind::STAGE_SECTION) {}
    };

    struct RsslSharedGlsl : RsslAstNode
    {
        std::string code;

        RsslSharedGlsl() : RsslAstNode(RsslNodeKind::SHARED_GLSL) {}
    };

    struct RsslProgram : RsslAstNode
    {
        int32_t version = 0;
        std::vector<std::unique_ptr<RsslAstNode>> declarations;

        RsslProgram() : RsslAstNode(RsslNodeKind::PROGRAM) {}
    };

    // ── Include resolver callback ───────────────────────────────────────

    using IncludeResolver = std::function<std::string(
        const std::string& path,
        const std::string& requestingFile)>;
}
