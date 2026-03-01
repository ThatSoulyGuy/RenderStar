#include <gtest/gtest.h>
#include "RenderStar/Client/Render/Shader/RsslCompiler.hpp"

using namespace RenderStar::Client::Render::Shader;

TEST(RsslCompilerEdgeCaseTest, DuplicateBindingsSameTypeFails)
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
}

TEST(RsslCompilerEdgeCaseTest, DuplicateBindingsDifferentTypesFails)
{
    auto result = RsslCompiler::Parse(
        "#rssl 1\n"
        "\n"
        "@uniform UBO : binding(0)\n"
        "{\n"
        "    mat4 m;\n"
        "}\n"
        "\n"
        "@sampler tex : binding(0)\n"
        "\n"
        "@stage vertex\n"
        "void main() {}\n");

    EXPECT_FALSE(result.IsValid());
    EXPECT_FALSE(result.errors.empty());
}

TEST(RsslCompilerEdgeCaseTest, DuplicateLocationInputFails)
{
    auto result = RsslCompiler::Parse(
        "#rssl 1\n"
        "\n"
        "@stage vertex\n"
        "\n"
        "@input a : location(0), vec3\n"
        "@input b : location(0), vec3\n"
        "\n"
        "void main() {}\n");

    EXPECT_FALSE(result.IsValid());
    EXPECT_FALSE(result.errors.empty());
}

TEST(RsslCompilerEdgeCaseTest, DuplicateLocationOutputFails)
{
    auto result = RsslCompiler::Parse(
        "#rssl 1\n"
        "\n"
        "@stage fragment\n"
        "\n"
        "@output a : location(0), vec4\n"
        "@output b : location(0), vec4\n"
        "\n"
        "void main() {}\n");

    EXPECT_FALSE(result.IsValid());
    EXPECT_FALSE(result.errors.empty());
}

TEST(RsslCompilerEdgeCaseTest, UnterminatedUniformBlockFails)
{
    auto result = RsslCompiler::Parse(
        "#rssl 1\n"
        "\n"
        "@uniform Broken : binding(0)\n"
        "{\n"
        "    mat4 m;\n"
        "\n"
        "@stage vertex\n"
        "void main() {}\n");

    EXPECT_FALSE(result.IsValid());
    EXPECT_FALSE(result.errors.empty());
}

TEST(RsslCompilerEdgeCaseTest, EmptyStageBodyCompiles)
{
    auto result = RsslCompiler::Compile(
        "#rssl 1\n"
        "\n"
        "@stage vertex\n"
        "\n"
        "@stage fragment\n"
        "void main() {}\n");

    EXPECT_TRUE(result.IsValid());
    EXPECT_FALSE(result.vertexGlsl.empty());
    EXPECT_TRUE(result.vertexGlsl.find("#version 450") != std::string::npos);
}

TEST(RsslCompilerEdgeCaseTest, InvalidStageTypeFails)
{
    auto result = RsslCompiler::Parse(
        "#rssl 1\n"
        "\n"
        "@stage geometry\n"
        "void main() {}\n");

    EXPECT_FALSE(result.IsValid());
    EXPECT_FALSE(result.errors.empty());
}

TEST(RsslCompilerEdgeCaseTest, DuplicateStageTypeFails)
{
    auto result = RsslCompiler::Parse(
        "#rssl 1\n"
        "\n"
        "@stage vertex\n"
        "void main() {}\n"
        "\n"
        "@stage vertex\n"
        "void main() {}\n");

    EXPECT_FALSE(result.IsValid());
    EXPECT_FALSE(result.errors.empty());
}

TEST(RsslCompilerEdgeCaseTest, InvalidVersionFails)
{
    auto result = RsslCompiler::Parse(
        "#rssl 99\n"
        "\n"
        "@stage vertex\n"
        "void main() {}\n");

    EXPECT_FALSE(result.IsValid());
    EXPECT_FALSE(result.errors.empty());
}

TEST(RsslCompilerEdgeCaseTest, VersionZeroFails)
{
    auto result = RsslCompiler::Parse(
        "#rssl 0\n"
        "\n"
        "@stage vertex\n"
        "void main() {}\n");

    EXPECT_FALSE(result.IsValid());
}

TEST(RsslCompilerEdgeCaseTest, ThreeUbosGetIndexedNames)
{
    auto result = RsslCompiler::Compile(
        "#rssl 1\n"
        "\n"
        "@uniform A : binding(0)\n"
        "{\n"
        "    mat4 a;\n"
        "}\n"
        "\n"
        "@uniform B : binding(1)\n"
        "{\n"
        "    mat4 b;\n"
        "}\n"
        "\n"
        "@uniform C : binding(2)\n"
        "{\n"
        "    mat4 c;\n"
        "}\n"
        "\n"
        "@stage vertex\n"
        "void main() {}\n");

    EXPECT_TRUE(result.IsValid());
    EXPECT_TRUE(result.vertexGlsl.find("} ubo;") != std::string::npos);
    EXPECT_TRUE(result.vertexGlsl.find("} b;") != std::string::npos);
    EXPECT_TRUE(result.vertexGlsl.find("} c;") != std::string::npos);
}

TEST(RsslCompilerEdgeCaseTest, SamplerAndUboOnSameBindingFails)
{
    auto result = RsslCompiler::Parse(
        "#rssl 1\n"
        "\n"
        "@uniform UBO : binding(1)\n"
        "{\n"
        "    mat4 m;\n"
        "}\n"
        "\n"
        "@sampler tex : binding(1)\n"
        "\n"
        "@stage vertex\n"
        "void main() {}\n");

    EXPECT_FALSE(result.IsValid());
    EXPECT_FALSE(result.errors.empty());
}

TEST(RsslCompilerEdgeCaseTest, ComputeWithVertexFails)
{
    auto result = RsslCompiler::Parse(
        "#rssl 1\n"
        "\n"
        "@stage compute\n"
        "void main() {}\n"
        "\n"
        "@stage vertex\n"
        "void main() {}\n");

    EXPECT_FALSE(result.IsValid());
    EXPECT_FALSE(result.errors.empty());
}

TEST(RsslCompilerEdgeCaseTest, SharedFunctionInComputeStage)
{
    auto result = RsslCompiler::Compile(
        "#rssl 1\n"
        "\n"
        "float helper(float x) { return x * 2.0; }\n"
        "\n"
        "@stage compute\n"
        "void main() {\n"
        "    float v = helper(1.0);\n"
        "}\n");

    EXPECT_TRUE(result.IsValid());
    EXPECT_TRUE(result.computeGlsl.find("float helper(float x)") != std::string::npos);
    EXPECT_TRUE(result.computeGlsl.find("helper(1.0)") != std::string::npos);
}

TEST(RsslCompilerEdgeCaseTest, WhitespaceInAnnotations)
{
    auto result = RsslCompiler::Parse(
        "#rssl 1\n"
        "\n"
        "@uniform  Foo  :  binding( 0 )\n"
        "{\n"
        "    vec4 bar;\n"
        "}\n"
        "\n"
        "@stage vertex\n"
        "void main() {}\n");

    EXPECT_TRUE(result.IsValid());
    EXPECT_EQ(result.uniformBlocks.size(), 1);
    EXPECT_EQ(result.uniformBlocks[0].name, "Foo");
    EXPECT_EQ(result.uniformBlocks[0].binding, 0);
}

TEST(RsslCompilerEdgeCaseTest, TrailingCommaInInputType)
{
    auto result = RsslCompiler::Parse(
        "#rssl 1\n"
        "\n"
        "@stage vertex\n"
        "\n"
        "@input pos : location(0), vec3,\n"
        "\n"
        "void main() {}\n");

    EXPECT_TRUE(result.IsValid());
    EXPECT_EQ(result.stages[0].inputs.size(), 0);
    EXPECT_TRUE(result.stages[0].glslBody.find("@input pos") != std::string::npos);
}

TEST(RsslCompilerEdgeCaseTest, MultipleInputLocationsAcrossStages)
{
    auto result = RsslCompiler::Parse(
        "#rssl 1\n"
        "\n"
        "@stage vertex\n"
        "\n"
        "@input inPos : location(0), vec3\n"
        "@output fragPos : location(0), vec3\n"
        "\n"
        "void main() { fragPos = inPos; }\n"
        "\n"
        "@stage fragment\n"
        "\n"
        "@input fragPos : location(0), vec3\n"
        "@output outColor : location(0), vec4\n"
        "\n"
        "void main() { outColor = vec4(fragPos, 1.0); }\n");

    EXPECT_TRUE(result.IsValid());
    EXPECT_EQ(result.stages.size(), 2);
    EXPECT_EQ(result.stages[0].inputs[0].location, 0);
    EXPECT_EQ(result.stages[1].inputs[0].location, 0);
}

TEST(RsslCompilerEdgeCaseTest, ComputeStageWithUniformAndSampler)
{
    auto result = RsslCompiler::Compile(
        "#rssl 1\n"
        "\n"
        "@uniform Params : binding(0)\n"
        "{\n"
        "    float time;\n"
        "}\n"
        "\n"
        "@sampler inputTex : binding(1)\n"
        "\n"
        "@stage compute\n"
        "void main() {\n"
        "    float t = ubo.time;\n"
        "}\n");

    EXPECT_TRUE(result.IsValid());
    EXPECT_TRUE(result.computeGlsl.find("uniform Params") != std::string::npos);
    EXPECT_TRUE(result.computeGlsl.find("} ubo;") != std::string::npos);
    EXPECT_TRUE(result.computeGlsl.find("sampler2D inputTex") != std::string::npos);
}
