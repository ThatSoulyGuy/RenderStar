#include <gtest/gtest.h>
#include "RenderStar/Client/Render/Shader/RsslCompiler.hpp"

using namespace RenderStar::Client::Render::Shader;

TEST(RsslCompilerTest, ParseVersionDirective)
{
    auto result = RsslCompiler::Parse("#rssl 1\n\n@stage vertex\nvoid main() {}\n");
    EXPECT_TRUE(result.IsValid());
    EXPECT_EQ(result.version, 1);
}

TEST(RsslCompilerTest, ParseMissingVersionFails)
{
    auto result = RsslCompiler::Parse("@stage vertex\nvoid main() {}\n");
    EXPECT_FALSE(result.IsValid());
    EXPECT_FALSE(result.errors.empty());
}

TEST(RsslCompilerTest, ParseEmptySourceFails)
{
    auto result = RsslCompiler::Parse("");
    EXPECT_FALSE(result.IsValid());
}

TEST(RsslCompilerTest, ParseNoStagesFails)
{
    auto result = RsslCompiler::Parse("#rssl 1\n");
    EXPECT_FALSE(result.IsValid());
}

TEST(RsslCompilerTest, ParseVertexStage)
{
    auto result = RsslCompiler::Parse(
        "#rssl 1\n"
        "\n"
        "@stage vertex\n"
        "\n"
        "void main() {\n"
        "    gl_Position = vec4(0.0);\n"
        "}\n");

    EXPECT_TRUE(result.IsValid());
    EXPECT_EQ(result.stages.size(), 1);
    EXPECT_EQ(result.stages[0].type, RsslStageType::VERTEX);
    EXPECT_TRUE(result.stages[0].glslBody.find("gl_Position") != std::string::npos);
}

TEST(RsslCompilerTest, ParseVertexAndFragmentStages)
{
    auto result = RsslCompiler::Parse(
        "#rssl 1\n"
        "\n"
        "@stage vertex\n"
        "void main() { gl_Position = vec4(0.0); }\n"
        "\n"
        "@stage fragment\n"
        "void main() { }\n");

    EXPECT_TRUE(result.IsValid());
    EXPECT_EQ(result.stages.size(), 2);
    EXPECT_TRUE(result.HasStage(RsslStageType::VERTEX));
    EXPECT_TRUE(result.HasStage(RsslStageType::FRAGMENT));
    EXPECT_FALSE(result.HasStage(RsslStageType::COMPUTE));
}

TEST(RsslCompilerTest, ParseUniformBlock)
{
    auto result = RsslCompiler::Parse(
        "#rssl 1\n"
        "\n"
        "@uniform UniformBufferObject : binding(0)\n"
        "{\n"
        "    mat4 model;\n"
        "    mat4 viewProjection;\n"
        "}\n"
        "\n"
        "@stage vertex\n"
        "void main() {}\n");

    EXPECT_TRUE(result.IsValid());
    EXPECT_EQ(result.uniformBlocks.size(), 1);
    EXPECT_EQ(result.uniformBlocks[0].name, "UniformBufferObject");
    EXPECT_EQ(result.uniformBlocks[0].binding, 0);
    EXPECT_TRUE(result.uniformBlocks[0].body.find("mat4 model;") != std::string::npos);
    EXPECT_TRUE(result.uniformBlocks[0].body.find("mat4 viewProjection;") != std::string::npos);
}

TEST(RsslCompilerTest, ParseMultipleUniformBlocks)
{
    auto result = RsslCompiler::Parse(
        "#rssl 1\n"
        "\n"
        "@uniform Camera : binding(0)\n"
        "{\n"
        "    mat4 view;\n"
        "}\n"
        "\n"
        "@uniform Model : binding(1)\n"
        "{\n"
        "    mat4 transform;\n"
        "}\n"
        "\n"
        "@stage vertex\n"
        "void main() {}\n");

    EXPECT_TRUE(result.IsValid());
    EXPECT_EQ(result.uniformBlocks.size(), 2);
    EXPECT_EQ(result.uniformBlocks[0].name, "Camera");
    EXPECT_EQ(result.uniformBlocks[0].binding, 0);
    EXPECT_EQ(result.uniformBlocks[1].name, "Model");
    EXPECT_EQ(result.uniformBlocks[1].binding, 1);
}

TEST(RsslCompilerTest, ParseSampler)
{
    auto result = RsslCompiler::Parse(
        "#rssl 1\n"
        "\n"
        "@sampler texSampler : binding(1)\n"
        "\n"
        "@stage vertex\n"
        "void main() {}\n");

    EXPECT_TRUE(result.IsValid());
    EXPECT_EQ(result.samplers.size(), 1);
    EXPECT_EQ(result.samplers[0].name, "texSampler");
    EXPECT_EQ(result.samplers[0].binding, 1);
    EXPECT_EQ(result.samplers[0].samplerType, "sampler2D");
}

TEST(RsslCompilerTest, ParseSampler3D)
{
    auto result = RsslCompiler::Parse(
        "#rssl 1\n"
        "\n"
        "@sampler3D volumeTex : binding(2)\n"
        "\n"
        "@stage vertex\n"
        "void main() {}\n");

    EXPECT_TRUE(result.IsValid());
    EXPECT_EQ(result.samplers[0].samplerType, "sampler3D");
}

TEST(RsslCompilerTest, ParseSamplerCube)
{
    auto result = RsslCompiler::Parse(
        "#rssl 1\n"
        "\n"
        "@samplerCube envMap : binding(3)\n"
        "\n"
        "@stage vertex\n"
        "void main() {}\n");

    EXPECT_TRUE(result.IsValid());
    EXPECT_EQ(result.samplers[0].samplerType, "samplerCube");
}

TEST(RsslCompilerTest, ParseStageInputs)
{
    auto result = RsslCompiler::Parse(
        "#rssl 1\n"
        "\n"
        "@stage vertex\n"
        "\n"
        "@input inPosition : location(0), vec3\n"
        "@input inColor : location(1), vec3\n"
        "\n"
        "void main() {}\n");

    EXPECT_TRUE(result.IsValid());
    EXPECT_EQ(result.stages[0].inputs.size(), 2);
    EXPECT_EQ(result.stages[0].inputs[0].name, "inPosition");
    EXPECT_EQ(result.stages[0].inputs[0].location, 0);
    EXPECT_EQ(result.stages[0].inputs[0].glslType, "vec3");
    EXPECT_TRUE(result.stages[0].inputs[0].isInput);
    EXPECT_EQ(result.stages[0].inputs[1].name, "inColor");
    EXPECT_EQ(result.stages[0].inputs[1].location, 1);
}

TEST(RsslCompilerTest, ParseStageOutputs)
{
    auto result = RsslCompiler::Parse(
        "#rssl 1\n"
        "\n"
        "@stage fragment\n"
        "\n"
        "@output outColor : location(0), vec4\n"
        "\n"
        "void main() {}\n");

    EXPECT_TRUE(result.IsValid());
    EXPECT_EQ(result.stages[0].outputs.size(), 1);
    EXPECT_EQ(result.stages[0].outputs[0].name, "outColor");
    EXPECT_EQ(result.stages[0].outputs[0].location, 0);
    EXPECT_EQ(result.stages[0].outputs[0].glslType, "vec4");
    EXPECT_FALSE(result.stages[0].outputs[0].isInput);
}

TEST(RsslCompilerTest, ParseSharedGlsl)
{
    auto result = RsslCompiler::Parse(
        "#rssl 1\n"
        "\n"
        "const float PI = 3.14159;\n"
        "\n"
        "@stage vertex\n"
        "void main() {}\n");

    EXPECT_TRUE(result.IsValid());
    EXPECT_TRUE(result.sharedGlsl.find("const float PI = 3.14159;") != std::string::npos);
}

TEST(RsslCompilerTest, CompileMinimalShader)
{
    auto result = RsslCompiler::Compile(
        "#rssl 1\n"
        "\n"
        "@stage vertex\n"
        "\n"
        "void main() {\n"
        "    gl_Position = vec4(0.0);\n"
        "}\n");

    EXPECT_TRUE(result.IsValid());
    EXPECT_FALSE(result.vertexGlsl.empty());
    EXPECT_TRUE(result.vertexGlsl.find("#version 450") != std::string::npos);
    EXPECT_TRUE(result.vertexGlsl.find("gl_Position") != std::string::npos);
}

TEST(RsslCompilerTest, CompileEmitsVersionDirective)
{
    auto result = RsslCompiler::Compile(
        "#rssl 1\n"
        "@stage vertex\n"
        "void main() {}\n");

    EXPECT_TRUE(result.IsValid());
    EXPECT_EQ(result.vertexGlsl.substr(0, 12), "#version 450");
}

TEST(RsslCompilerTest, CompileEmitsUboWithInstanceName)
{
    auto result = RsslCompiler::Compile(
        "#rssl 1\n"
        "\n"
        "@uniform UniformBufferObject : binding(0)\n"
        "{\n"
        "    mat4 model;\n"
        "}\n"
        "\n"
        "@stage vertex\n"
        "void main() {}\n");

    EXPECT_TRUE(result.IsValid());
    EXPECT_TRUE(result.vertexGlsl.find("layout(std140, binding = 0) uniform UniformBufferObject {") != std::string::npos);
    EXPECT_TRUE(result.vertexGlsl.find("} ubo;") != std::string::npos);
}

TEST(RsslCompilerTest, CompileMultipleUbosGetIndexedNames)
{
    auto result = RsslCompiler::Compile(
        "#rssl 1\n"
        "\n"
        "@uniform Camera : binding(0)\n"
        "{\n"
        "    mat4 view;\n"
        "}\n"
        "\n"
        "@uniform Model : binding(1)\n"
        "{\n"
        "    mat4 transform;\n"
        "}\n"
        "\n"
        "@stage vertex\n"
        "void main() {}\n");

    EXPECT_TRUE(result.IsValid());
    EXPECT_TRUE(result.vertexGlsl.find("} ubo;") != std::string::npos);
    EXPECT_TRUE(result.vertexGlsl.find("} model;") != std::string::npos);
}

TEST(RsslCompilerTest, CompileEmitsSampler)
{
    auto result = RsslCompiler::Compile(
        "#rssl 1\n"
        "\n"
        "@sampler texSampler : binding(1)\n"
        "\n"
        "@stage fragment\n"
        "void main() {}\n");

    EXPECT_TRUE(result.IsValid());
    EXPECT_TRUE(result.fragmentGlsl.find("layout(binding = 1) uniform sampler2D texSampler;") != std::string::npos);
}

TEST(RsslCompilerTest, CompileEmitsInputsAndOutputs)
{
    auto result = RsslCompiler::Compile(
        "#rssl 1\n"
        "\n"
        "@stage vertex\n"
        "\n"
        "@input inPosition : location(0), vec3\n"
        "@output fragColor : location(0), vec3\n"
        "\n"
        "void main() {\n"
        "    fragColor = inPosition;\n"
        "}\n");

    EXPECT_TRUE(result.IsValid());
    EXPECT_TRUE(result.vertexGlsl.find("layout(location = 0) in vec3 inPosition;") != std::string::npos);
    EXPECT_TRUE(result.vertexGlsl.find("layout(location = 0) out vec3 fragColor;") != std::string::npos);
}

TEST(RsslCompilerTest, CompileSharedDeclarationsInAllStages)
{
    auto result = RsslCompiler::Compile(
        "#rssl 1\n"
        "\n"
        "@uniform UBO : binding(0)\n"
        "{\n"
        "    mat4 mvp;\n"
        "}\n"
        "\n"
        "@sampler tex : binding(1)\n"
        "\n"
        "@stage vertex\n"
        "void main() {}\n"
        "\n"
        "@stage fragment\n"
        "void main() {}\n");

    EXPECT_TRUE(result.IsValid());
    EXPECT_TRUE(result.vertexGlsl.find("uniform UBO") != std::string::npos);
    EXPECT_TRUE(result.vertexGlsl.find("sampler2D tex") != std::string::npos);
    EXPECT_TRUE(result.fragmentGlsl.find("uniform UBO") != std::string::npos);
    EXPECT_TRUE(result.fragmentGlsl.find("sampler2D tex") != std::string::npos);
}

TEST(RsslCompilerTest, CompileSharedGlslInAllStages)
{
    auto result = RsslCompiler::Compile(
        "#rssl 1\n"
        "\n"
        "const float PI = 3.14159;\n"
        "\n"
        "@stage vertex\n"
        "void main() {}\n"
        "\n"
        "@stage fragment\n"
        "void main() {}\n");

    EXPECT_TRUE(result.IsValid());
    EXPECT_TRUE(result.vertexGlsl.find("const float PI = 3.14159;") != std::string::npos);
    EXPECT_TRUE(result.fragmentGlsl.find("const float PI = 3.14159;") != std::string::npos);
}

TEST(RsslCompilerTest, CompileInvalidSourceReturnsErrors)
{
    auto result = RsslCompiler::Compile("this is not valid rssl");
    EXPECT_FALSE(result.IsValid());
    EXPECT_FALSE(result.errors.empty());
}

TEST(RsslCompilerTest, CompileComputeStage)
{
    auto result = RsslCompiler::Compile(
        "#rssl 1\n"
        "\n"
        "@stage compute\n"
        "\n"
        "void main() {}\n");

    EXPECT_TRUE(result.IsValid());
    EXPECT_FALSE(result.computeGlsl.empty());
    EXPECT_TRUE(result.computeGlsl.find("#version 450") != std::string::npos);
    EXPECT_TRUE(result.vertexGlsl.empty());
    EXPECT_TRUE(result.fragmentGlsl.empty());
}

TEST(RsslCompilerTest, CompileSceneGeometryMatchesTestVert)
{
    std::string rssl =
        "#rssl 1\n"
        "\n"
        "@uniform UniformBufferObject : binding(0)\n"
        "{\n"
        "    mat4 model;\n"
        "    mat4 viewProjection;\n"
        "    vec4 colorTint;\n"
        "}\n"
        "\n"
        "@sampler texSampler : binding(1)\n"
        "\n"
        "@stage vertex\n"
        "\n"
        "@input inPosition : location(0), vec3\n"
        "@input inColor : location(1), vec3\n"
        "@input inTexCoord : location(2), vec2\n"
        "\n"
        "@output fragColor : location(0), vec3\n"
        "@output tintColor : location(1), vec4\n"
        "@output fragTexCoord : location(2), vec2\n"
        "\n"
        "void main() {\n"
        "    gl_Position = ubo.viewProjection * ubo.model * vec4(inPosition, 1.0);\n"
        "    fragColor = inColor;\n"
        "    tintColor = ubo.colorTint;\n"
        "    fragTexCoord = inTexCoord;\n"
        "}\n"
        "\n"
        "@stage fragment\n"
        "\n"
        "@input fragColor : location(0), vec3\n"
        "@input tintColor : location(1), vec4\n"
        "@input fragTexCoord : location(2), vec2\n"
        "\n"
        "@output outColor : location(0), vec4\n"
        "\n"
        "void main() {\n"
        "    vec4 texColor = texture(texSampler, fragTexCoord);\n"
        "    vec3 baseColor = fragColor * texColor.rgb;\n"
        "    vec3 finalColor = mix(baseColor, tintColor.rgb, tintColor.a);\n"
        "    outColor = vec4(finalColor, 1.0);\n"
        "}\n";

    auto result = RsslCompiler::Compile(rssl);
    EXPECT_TRUE(result.IsValid());

    EXPECT_TRUE(result.vertexGlsl.find("#version 450") != std::string::npos);
    EXPECT_TRUE(result.vertexGlsl.find("layout(std140, binding = 0) uniform UniformBufferObject {") != std::string::npos);
    EXPECT_TRUE(result.vertexGlsl.find("    mat4 model;") != std::string::npos);
    EXPECT_TRUE(result.vertexGlsl.find("    mat4 viewProjection;") != std::string::npos);
    EXPECT_TRUE(result.vertexGlsl.find("    vec4 colorTint;") != std::string::npos);
    EXPECT_TRUE(result.vertexGlsl.find("} ubo;") != std::string::npos);
    EXPECT_TRUE(result.vertexGlsl.find("layout(binding = 1) uniform sampler2D texSampler;") != std::string::npos);
    EXPECT_TRUE(result.vertexGlsl.find("layout(location = 0) in vec3 inPosition;") != std::string::npos);
    EXPECT_TRUE(result.vertexGlsl.find("layout(location = 1) in vec3 inColor;") != std::string::npos);
    EXPECT_TRUE(result.vertexGlsl.find("layout(location = 2) in vec2 inTexCoord;") != std::string::npos);
    EXPECT_TRUE(result.vertexGlsl.find("layout(location = 0) out vec3 fragColor;") != std::string::npos);
    EXPECT_TRUE(result.vertexGlsl.find("layout(location = 1) out vec4 tintColor;") != std::string::npos);
    EXPECT_TRUE(result.vertexGlsl.find("layout(location = 2) out vec2 fragTexCoord;") != std::string::npos);
    EXPECT_TRUE(result.vertexGlsl.find("gl_Position = ubo.viewProjection * ubo.model * vec4(inPosition, 1.0);") != std::string::npos);

    EXPECT_TRUE(result.fragmentGlsl.find("#version 450") != std::string::npos);
    EXPECT_TRUE(result.fragmentGlsl.find("layout(std140, binding = 0) uniform UniformBufferObject {") != std::string::npos);
    EXPECT_TRUE(result.fragmentGlsl.find("} ubo;") != std::string::npos);
    EXPECT_TRUE(result.fragmentGlsl.find("layout(binding = 1) uniform sampler2D texSampler;") != std::string::npos);
    EXPECT_TRUE(result.fragmentGlsl.find("layout(location = 0) in vec3 fragColor;") != std::string::npos);
    EXPECT_TRUE(result.fragmentGlsl.find("layout(location = 0) out vec4 outColor;") != std::string::npos);
    EXPECT_TRUE(result.fragmentGlsl.find("texture(texSampler, fragTexCoord)") != std::string::npos);
}

TEST(RsslCompilerTest, ParseResultGetStageReturnsNullForMissing)
{
    auto result = RsslCompiler::Parse(
        "#rssl 1\n"
        "@stage vertex\n"
        "void main() {}\n");

    EXPECT_TRUE(result.IsValid());
    EXPECT_NE(result.GetStage(RsslStageType::VERTEX), nullptr);
    EXPECT_EQ(result.GetStage(RsslStageType::FRAGMENT), nullptr);
    EXPECT_EQ(result.GetStage(RsslStageType::COMPUTE), nullptr);
}

TEST(RsslCompilerTest, CompileFromParsedResult)
{
    auto parsed = RsslCompiler::Parse(
        "#rssl 1\n"
        "@stage vertex\n"
        "void main() { gl_Position = vec4(0.0); }\n");

    EXPECT_TRUE(parsed.IsValid());

    auto result = RsslCompiler::Compile(parsed);
    EXPECT_TRUE(result.IsValid());
    EXPECT_TRUE(result.vertexGlsl.find("#version 450") != std::string::npos);
}

TEST(RsslCompilerTest, FullscreenShaderNoVertexInputs)
{
    auto result = RsslCompiler::Compile(
        "#rssl 1\n"
        "\n"
        "@sampler sceneColor : binding(0)\n"
        "\n"
        "@stage vertex\n"
        "\n"
        "@output fragTexCoord : location(0), vec2\n"
        "\n"
        "void main() {\n"
        "    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);\n"
        "    fragTexCoord = vec2(0.0);\n"
        "}\n"
        "\n"
        "@stage fragment\n"
        "\n"
        "@input fragTexCoord : location(0), vec2\n"
        "\n"
        "@output outColor : location(0), vec4\n"
        "\n"
        "void main() {\n"
        "    outColor = texture(sceneColor, fragTexCoord);\n"
        "}\n");

    EXPECT_TRUE(result.IsValid());
    EXPECT_TRUE(result.vertexGlsl.find("layout(location = 0) out vec2 fragTexCoord;") != std::string::npos);
    EXPECT_TRUE(result.vertexGlsl.find("in ") == std::string::npos || result.vertexGlsl.find("layout(location") == result.vertexGlsl.find("layout(location = 0) out"));
    EXPECT_TRUE(result.fragmentGlsl.find("layout(location = 0) in vec2 fragTexCoord;") != std::string::npos);
    EXPECT_TRUE(result.fragmentGlsl.find("texture(sceneColor, fragTexCoord)") != std::string::npos);
}

// ── New feature tests ──────────────────────────────────────────────────

TEST(RsslCompilerTest, ParseIncludeDirective)
{
    auto resolver = [](const std::string& path, const std::string&) -> std::string
    {
        if (path == "common.rssl")
        {
            return
                "@uniform Globals : binding(0)\n"
                "{\n"
                "    mat4 viewProj;\n"
                "}\n";
        }

        throw std::runtime_error("File not found: " + path);
    };

    auto result = RsslCompiler::Parse(
        "#rssl 1\n"
        "\n"
        "#include \"common.rssl\"\n"
        "\n"
        "@stage vertex\n"
        "void main() { gl_Position = ubo.viewProj * vec4(0.0); }\n",
        resolver, "test.rssl");

    EXPECT_TRUE(result.IsValid());
    EXPECT_EQ(result.uniformBlocks.size(), 1);
    EXPECT_EQ(result.uniformBlocks[0].name, "Globals");
    EXPECT_EQ(result.uniformBlocks[0].binding, 0);
}

TEST(RsslCompilerTest, ParseIncludeCircularDetection)
{
    auto resolver = [](const std::string& path, const std::string&) -> std::string
    {
        if (path == "a.rssl")
            return "#include \"b.rssl\"\n";
        if (path == "b.rssl")
            return "#include \"a.rssl\"\n";

        throw std::runtime_error("File not found: " + path);
    };

    auto result = RsslCompiler::Parse(
        "#rssl 1\n"
        "#include \"a.rssl\"\n"
        "@stage vertex\n"
        "void main() {}\n",
        resolver, "root.rssl");

    EXPECT_TRUE(result.IsValid());
}

TEST(RsslCompilerTest, CompileNestedInclude)
{
    auto resolver = [](const std::string& path, const std::string&) -> std::string
    {
        if (path == "outer.rssl")
        {
            return "#include \"inner.rssl\"\n"
                   "@sampler outerTex : binding(1)\n";
        }
        if (path == "inner.rssl")
        {
            return
                "@uniform Camera : binding(0)\n"
                "{\n"
                "    mat4 vp;\n"
                "}\n";
        }

        throw std::runtime_error("File not found: " + path);
    };

    auto result = RsslCompiler::Compile(
        "#rssl 1\n"
        "#include \"outer.rssl\"\n"
        "@stage vertex\n"
        "void main() { gl_Position = ubo.vp * vec4(0.0); }\n",
        resolver, "test.rssl");

    EXPECT_TRUE(result.IsValid());
    EXPECT_TRUE(result.vertexGlsl.find("uniform Camera") != std::string::npos);
    EXPECT_TRUE(result.vertexGlsl.find("sampler2D outerTex") != std::string::npos);
}

TEST(RsslCompilerTest, CompileIncludeOnce)
{
    int resolveCount = 0;

    auto resolver = [&](const std::string& path, const std::string&) -> std::string
    {
        if (path == "shared.rssl")
        {
            resolveCount++;
            return
                "@uniform Shared : binding(0)\n"
                "{\n"
                "    float time;\n"
                "}\n";
        }

        throw std::runtime_error("File not found: " + path);
    };

    auto result = RsslCompiler::Parse(
        "#rssl 1\n"
        "#include \"shared.rssl\"\n"
        "#include \"shared.rssl\"\n"
        "@stage vertex\n"
        "void main() {}\n",
        resolver, "test.rssl");

    EXPECT_TRUE(result.IsValid());
    EXPECT_EQ(result.uniformBlocks.size(), 1);
    EXPECT_EQ(resolveCount, 1);
}

TEST(RsslCompilerTest, ParseDefineDirective)
{
    auto resolver = [](const std::string&, const std::string&) -> std::string
    {
        return "";
    };

    auto result = RsslCompiler::Compile(
        "#rssl 1\n"
        "\n"
        "#define MAX_LIGHTS 4\n"
        "\n"
        "@stage vertex\n"
        "void main() {\n"
        "    for (int i = 0; i < MAX_LIGHTS; i++) {}\n"
        "}\n",
        resolver, "test.rssl");

    EXPECT_TRUE(result.IsValid());
    EXPECT_TRUE(result.vertexGlsl.find("MAX_LIGHTS") == std::string::npos);
    EXPECT_TRUE(result.vertexGlsl.find("i < 4") != std::string::npos);
}

TEST(RsslCompilerTest, CompileErrorHasLineNumbers)
{
    auto result = RsslCompiler::Parse(
        "#rssl 1\n"
        "\n"
        "@uniform A : binding(0)\n"
        "{\n"
        "    mat4 m;\n"
        "}\n"
        "\n"
        "@uniform B : binding(0)\n"
        "{\n"
        "    mat4 n;\n"
        "}\n"
        "\n"
        "@stage vertex\n"
        "void main() {}\n");

    EXPECT_FALSE(result.IsValid());
    EXPECT_FALSE(result.errors.empty());
    EXPECT_TRUE(result.errors[0].find("error:") != std::string::npos);
}
