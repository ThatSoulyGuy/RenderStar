#include <gtest/gtest.h>
#include "RenderStar/Client/Render/Shader/RsslCompiler.hpp"

using namespace RenderStar::Client::Render::Shader;

static const char* SIMPLE_RSSL =
    "#rssl 1\n"
    "\n"
    "@uniform Camera : binding(0)\n"
    "{\n"
    "    mat4 viewProjection;\n"
    "}\n"
    "\n"
    "@sampler tex : binding(1)\n"
    "\n"
    "@stage vertex\n"
    "\n"
    "@input inPos : location(0), vec3\n"
    "@output fragUv : location(0), vec2\n"
    "\n"
    "void main() {\n"
    "    gl_Position = ubo.viewProjection * vec4(inPos, 1.0);\n"
    "    fragUv = inPos.xy;\n"
    "}\n"
    "\n"
    "@stage fragment\n"
    "\n"
    "@input fragUv : location(0), vec2\n"
    "@output outColor : location(0), vec4\n"
    "\n"
    "void main() {\n"
    "    outColor = texture(tex, fragUv);\n"
    "}\n";

TEST(RsslBackendEmitterTest, VulkanGlslEmitsVersion450)
{
    auto result = RsslCompiler::Compile(SIMPLE_RSSL, RsslTarget::VULKAN_GLSL);

    EXPECT_TRUE(result.IsValid());
    EXPECT_TRUE(result.vertexGlsl.find("#version 450") != std::string::npos);
    EXPECT_TRUE(result.fragmentGlsl.find("#version 450") != std::string::npos);
}

TEST(RsslBackendEmitterTest, OpenGlGlslEmitsVersion410Core)
{
    auto result = RsslCompiler::Compile(SIMPLE_RSSL, RsslTarget::OPENGL_GLSL);

    EXPECT_TRUE(result.IsValid());
    EXPECT_TRUE(result.vertexGlsl.find("#version 410 core") != std::string::npos);
    EXPECT_TRUE(result.fragmentGlsl.find("#version 410 core") != std::string::npos);
    EXPECT_TRUE(result.vertexGlsl.find("#version 450") == std::string::npos);
}

TEST(RsslBackendEmitterTest, OpenGlRenamesVertexIndex)
{
    auto result = RsslCompiler::Compile(
        "#rssl 1\n"
        "@stage vertex\n"
        "void main() { int id = gl_VertexIndex; }\n",
        RsslTarget::OPENGL_GLSL);

    EXPECT_TRUE(result.IsValid());
    EXPECT_TRUE(result.vertexGlsl.find("gl_VertexID") != std::string::npos);
    EXPECT_TRUE(result.vertexGlsl.find("gl_VertexIndex") == std::string::npos);
}

TEST(RsslBackendEmitterTest, OpenGlRenamesInstanceIndex)
{
    auto result = RsslCompiler::Compile(
        "#rssl 1\n"
        "@stage vertex\n"
        "void main() { int id = gl_InstanceIndex; }\n",
        RsslTarget::OPENGL_GLSL);

    EXPECT_TRUE(result.IsValid());
    EXPECT_TRUE(result.vertexGlsl.find("gl_InstanceID") != std::string::npos);
    EXPECT_TRUE(result.vertexGlsl.find("gl_InstanceIndex") == std::string::npos);
}

TEST(RsslBackendEmitterTest, VulkanPreservesVulkanBuiltins)
{
    auto result = RsslCompiler::Compile(
        "#rssl 1\n"
        "@stage vertex\n"
        "void main() { int id = gl_VertexIndex + gl_InstanceIndex; }\n",
        RsslTarget::VULKAN_GLSL);

    EXPECT_TRUE(result.IsValid());
    EXPECT_TRUE(result.vertexGlsl.find("gl_VertexIndex") != std::string::npos);
    EXPECT_TRUE(result.vertexGlsl.find("gl_InstanceIndex") != std::string::npos);
}

TEST(RsslBackendEmitterTest, OpenGlFlipsShadowY)
{
    auto result = RsslCompiler::Compile(
        "#rssl 1\n"
        "@stage fragment\n"
        "@output outColor : location(0), vec4\n"
        "void main() {\n"
        "    vec2 coord = fragPos.xy * vec2(0.5, -0.5) + vec2(0.5);\n"
        "    outColor = vec4(coord, 0.0, 1.0);\n"
        "}\n",
        RsslTarget::OPENGL_GLSL);

    EXPECT_TRUE(result.IsValid());
    EXPECT_TRUE(result.fragmentGlsl.find("vec2(0.5, 0.5)") != std::string::npos);
    EXPECT_TRUE(result.fragmentGlsl.find("vec2(0.5, -0.5)") == std::string::npos);
}

TEST(RsslBackendEmitterTest, VulkanPreservesShadowY)
{
    auto result = RsslCompiler::Compile(
        "#rssl 1\n"
        "@stage fragment\n"
        "@output outColor : location(0), vec4\n"
        "void main() {\n"
        "    vec2 coord = fragPos.xy * vec2(0.5, -0.5) + vec2(0.5);\n"
        "    outColor = vec4(coord, 0.0, 1.0);\n"
        "}\n",
        RsslTarget::VULKAN_GLSL);

    EXPECT_TRUE(result.IsValid());
    EXPECT_TRUE(result.fragmentGlsl.find("vec2(0.5, -0.5)") != std::string::npos);
}

TEST(RsslBackendEmitterTest, BothTargetsEmitUniformsIdentically)
{
    auto vulkan = RsslCompiler::Compile(SIMPLE_RSSL, RsslTarget::VULKAN_GLSL);
    auto opengl = RsslCompiler::Compile(SIMPLE_RSSL, RsslTarget::OPENGL_GLSL);

    EXPECT_TRUE(vulkan.IsValid());
    EXPECT_TRUE(opengl.IsValid());

    EXPECT_TRUE(vulkan.vertexGlsl.find("layout(std140, binding = 0) uniform Camera") != std::string::npos);
    EXPECT_TRUE(opengl.vertexGlsl.find("layout(std140, binding = 0) uniform Camera") != std::string::npos);

    EXPECT_TRUE(vulkan.fragmentGlsl.find("sampler2D tex") != std::string::npos);
    EXPECT_TRUE(opengl.fragmentGlsl.find("sampler2D tex") != std::string::npos);
}

TEST(RsslBackendEmitterTest, DefaultTargetIsVulkan)
{
    auto defaultResult = RsslCompiler::Compile(SIMPLE_RSSL);
    auto vulkanResult = RsslCompiler::Compile(SIMPLE_RSSL, RsslTarget::VULKAN_GLSL);

    EXPECT_EQ(defaultResult.vertexGlsl, vulkanResult.vertexGlsl);
    EXPECT_EQ(defaultResult.fragmentGlsl, vulkanResult.fragmentGlsl);
}
