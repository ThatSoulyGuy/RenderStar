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
    EXPECT_TRUE(result.find("#version 410 core") != std::string::npos);
    EXPECT_TRUE(result.find("#version 450") == std::string::npos);
}

TEST(GlslTransformerTest, TransformUboBindings)
{
    std::string source = "#version 450\nlayout(binding = 0) uniform UniformBufferObject {\n    mat4 model;\n} ubo;\nvoid main() {}";
    auto result = GlslTransformer::Transform450To410(source, ShaderType::VERTEX);
    EXPECT_TRUE(result.find("layout(std140) uniform UniformBufferObject") != std::string::npos);
    EXPECT_TRUE(result.find("layout(binding = 0)") == std::string::npos);
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
    EXPECT_TRUE(result.find("#version 410 core") != std::string::npos);
    EXPECT_TRUE(result.find("layout(std140) uniform") != std::string::npos);
    EXPECT_TRUE(result.find("layout(location = 0) in vec3 inPosition") != std::string::npos);
}
