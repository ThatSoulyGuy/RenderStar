#include <gtest/gtest.h>
#include "RenderStar/Client/Render/Shader/GlslTransformer.hpp"

using namespace RenderStar::Client::Render::Shader;

TEST(GlslTransformerTest, CanTransform450)
{
    EXPECT_TRUE(GlslTransformer::CanTransform("#version 450\nvoid main() {}"));
}

TEST(GlslTransformerTest, CanTransform460)
{
    EXPECT_TRUE(GlslTransformer::CanTransform("#version 460 core\nvoid main() {}"));
}

TEST(GlslTransformerTest, CannotTransform330)
{
    EXPECT_FALSE(GlslTransformer::CanTransform("#version 330 core\nvoid main() {}"));
}

TEST(GlslTransformerTest, CannotTransform410)
{
    EXPECT_FALSE(GlslTransformer::CanTransform("#version 410 core\nvoid main() {}"));
}

TEST(GlslTransformerTest, CannotTransformEmpty)
{
    EXPECT_FALSE(GlslTransformer::CanTransform(""));
}

TEST(GlslTransformerTest, TransformVersionDirective)
{
    std::string source = "#version 450\nvoid main() {}";
    auto result = GlslTransformer::Transform450To410(source, ShaderType::VERTEX);
    EXPECT_TRUE(result.find("#version 450 core") != std::string::npos);
}

TEST(GlslTransformerTest, TransformUboBindings)
{
    std::string source = "#version 450\nlayout(binding = 0) uniform UniformBufferObject {\n    mat4 model;\n} ubo;\nvoid main() {}";
    auto result = GlslTransformer::Transform450To410(source, ShaderType::VERTEX);
    EXPECT_TRUE(result.find("layout(binding = 0, std140) uniform UniformBufferObject {") != std::string::npos);
}

TEST(GlslTransformerTest, SamplerBindingPreserved)
{
    std::string source = "#version 450\nlayout(binding = 1) uniform sampler2D texSampler;\nvoid main() {}";
    auto result = GlslTransformer::Transform450To410(source, ShaderType::FRAGMENT);
    EXPECT_TRUE(result.find("layout(binding = 1) uniform sampler2D texSampler;") != std::string::npos);
}

TEST(GlslTransformerTest, UboAndSamplerCombined)
{
    std::string source =
        "#version 450\n"
        "layout(binding = 0) uniform UniformBufferObject {\n"
        "    mat4 model;\n"
        "} ubo;\n"
        "layout(binding = 1) uniform sampler2D texSampler;\n"
        "void main() {}\n";
    auto result = GlslTransformer::Transform450To410(source, ShaderType::FRAGMENT);
    EXPECT_TRUE(result.find("layout(binding = 0, std140) uniform UniformBufferObject {") != std::string::npos);
    EXPECT_TRUE(result.find("layout(binding = 1) uniform sampler2D texSampler;") != std::string::npos);
}

TEST(GlslTransformerTest, EmptySourceReturnsEmpty)
{
    auto result = GlslTransformer::Transform450To410("", ShaderType::VERTEX);
    EXPECT_TRUE(result.empty());
}

TEST(GlslTransformerTest, Non450ReturnedUnchanged)
{
    std::string source = "#version 330 core\nvoid main() {}";
    auto result = GlslTransformer::Transform450To410(source, ShaderType::VERTEX);
    EXPECT_EQ(result, source);
}

TEST(GlslTransformerTest, FullShaderRoundtrip)
{
    std::string source =
        "#version 450\n"
        "\n"
        "layout(binding = 0) uniform UniformBufferObject {\n"
        "    mat4 model;\n"
        "    mat4 viewProjection;\n"
        "    vec4 colorTint;\n"
        "} ubo;\n"
        "\n"
        "layout(location = 0) in vec3 inPosition;\n"
        "layout(location = 1) in vec3 inColor;\n"
        "layout(location = 2) in vec2 inTexCoord;\n"
        "\n"
        "layout(location = 0) out vec3 fragColor;\n"
        "\n"
        "void main() {\n"
        "    gl_Position = ubo.viewProjection * ubo.model * vec4(inPosition, 1.0);\n"
        "    fragColor = inColor * ubo.colorTint.rgb;\n"
        "}\n";

    auto result = GlslTransformer::Transform450To410(source, ShaderType::VERTEX);
    EXPECT_TRUE(result.find("#version 450 core") != std::string::npos);
    EXPECT_TRUE(result.find("std140") != std::string::npos);
    EXPECT_TRUE(result.find("layout(location = 0) in vec3 inPosition") != std::string::npos);
}
