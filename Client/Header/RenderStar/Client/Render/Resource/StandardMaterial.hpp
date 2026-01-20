#pragma once

#include "RenderStar/Client/Render/Resource/IMaterial.hpp"
#include "RenderStar/Client/Render/Resource/IShaderProgram.hpp"
#include "RenderStar/Client/Render/Resource/IBufferHandle.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>
#include <string>

namespace RenderStar::Client::Render
{
    struct MVPMatrices
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 projection;
    };

    class StandardMaterial : public IMaterial
    {
    public:

        StandardMaterial();
        ~StandardMaterial() override;

        void Initialize(IShaderProgram* shader, IBufferHandle* uniformBuffer);

        IShaderProgram* GetShader() const override;
        const UniformLayout& GetUniformLayout() const override;

        void SetMatrix(const std::string& name, const glm::mat4& matrix) override;
        void SetVector(const std::string& name, const glm::vec4& vector) override;
        void SetFloat(const std::string& name, float value) override;
        void SetInt(const std::string& name, int32_t value) override;

        void UploadUniforms() override;

        bool IsValid() const override;

        void SetModelMatrix(const glm::mat4& model);
        void SetViewMatrix(const glm::mat4& view);
        void SetProjectionMatrix(const glm::mat4& projection);

        const MVPMatrices& GetMVPMatrices() const;

    private:

        IShaderProgram* shader;
        IBufferHandle* uniformBuffer;
        UniformLayout uniformLayout;
        MVPMatrices mvpMatrices;
        bool dirty;
    };
}
