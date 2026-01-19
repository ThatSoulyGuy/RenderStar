#pragma once

#include <string>
#include <glm/glm.hpp>

namespace RenderStar::Client::Render
{
    class IShaderProgram
    {
    public:

        virtual ~IShaderProgram() = default;

        virtual void Bind() = 0;
        virtual void Unbind() = 0;

        virtual bool IsValid() const = 0;

        virtual void SetUniform(const std::string& name, int32_t value) = 0;
        virtual void SetUniform(const std::string& name, float value) = 0;
        virtual void SetUniform(const std::string& name, const glm::vec2& value) = 0;
        virtual void SetUniform(const std::string& name, const glm::vec3& value) = 0;
        virtual void SetUniform(const std::string& name, const glm::vec4& value) = 0;
        virtual void SetUniform(const std::string& name, const glm::mat3& value) = 0;
        virtual void SetUniform(const std::string& name, const glm::mat4& value) = 0;

        virtual int32_t GetUniformLocation(const std::string& name) const = 0;
    };
}
