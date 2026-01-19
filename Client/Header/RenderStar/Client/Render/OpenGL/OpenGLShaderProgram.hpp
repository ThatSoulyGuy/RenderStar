#pragma once

#include "RenderStar/Client/Render/Resource/IShaderProgram.hpp"
#include <cstdint>
#include <string>
#include <memory>
#include <spdlog/spdlog.h>

namespace RenderStar::Client::Render::OpenGL
{
    using ShaderProgramHandle = uint32_t;

    class OpenGLShaderProgram : public IShaderProgram
    {
    public:

        static constexpr ShaderProgramHandle INVALID_PROGRAM = 0;

        OpenGLShaderProgram();

        ~OpenGLShaderProgram() override;

        bool CompileFromSource(const std::string& vertexSource, const std::string& fragmentSource);

        void Bind() override;

        void Unbind() override;

        ShaderProgramHandle GetHandle() const;

        bool IsValid() const override;

        void SetUniformBlockBinding(const std::string& blockName, uint32_t bindingPoint);

        void SetUniform(const std::string& name, int32_t value) override;
        void SetUniform(const std::string& name, float value) override;
        void SetUniform(const std::string& name, const glm::vec2& value) override;
        void SetUniform(const std::string& name, const glm::vec3& value) override;
        void SetUniform(const std::string& name, const glm::vec4& value) override;
        void SetUniform(const std::string& name, const glm::mat3& value) override;
        void SetUniform(const std::string& name, const glm::mat4& value) override;

        int32_t GetUniformLocation(const std::string& name) const override;

    private:

        bool CheckShaderCompilation(uint32_t shader, const std::string& type);

        bool CheckProgramLinking();

        std::shared_ptr<spdlog::logger> logger;
        ShaderProgramHandle programHandle;
    };
}
