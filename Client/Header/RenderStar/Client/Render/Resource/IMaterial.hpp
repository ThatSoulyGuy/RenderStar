#pragma once

#include "RenderStar/Client/Render/Resource/IShaderProgram.hpp"
#include "RenderStar/Client/Render/Resource/UniformLayout.hpp"
#include <glm/glm.hpp>
#include <string>

namespace RenderStar::Client::Render
{
    class IMaterial
    {
    public:

        virtual ~IMaterial() = default;

        virtual IShaderProgram* GetShader() const = 0;
        virtual const UniformLayout& GetUniformLayout() const = 0;

        virtual void SetMatrix(const std::string& name, const glm::mat4& matrix) = 0;
        virtual void SetVector(const std::string& name, const glm::vec4& vector) = 0;
        virtual void SetFloat(const std::string& name, float value) = 0;
        virtual void SetInt(const std::string& name, int32_t value) = 0;

        virtual void UploadUniforms() = 0;

        virtual bool IsValid() const = 0;
    };
}
