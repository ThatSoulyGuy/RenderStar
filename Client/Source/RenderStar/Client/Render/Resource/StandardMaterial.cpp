#include "RenderStar/Client/Render/Resource/StandardMaterial.hpp"

namespace RenderStar::Client::Render
{
    StandardMaterial::StandardMaterial()
        : shader(nullptr)
        , uniformBuffer(nullptr)
        , uniformLayout(UniformLayout::ForMVP())
        , mvpMatrices{}
        , dirty(true)
    {
        mvpMatrices.model = glm::mat4(1.0f);
        mvpMatrices.view = glm::mat4(1.0f);
        mvpMatrices.projection = glm::mat4(1.0f);
    }

    StandardMaterial::~StandardMaterial() = default;

    void StandardMaterial::Initialize(IShaderProgram* shaderProgram, IBufferHandle* buffer)
    {
        shader = shaderProgram;
        uniformBuffer = buffer;
    }

    void StandardMaterial::Bind()
    {
        if (shader)
            shader->Bind();

        if (uniformBuffer)
            uniformBuffer->Bind();
    }

    void StandardMaterial::Unbind()
    {
        if (uniformBuffer)
            uniformBuffer->Unbind();

        if (shader)
            shader->Unbind();
    }

    IShaderProgram* StandardMaterial::GetShader() const
    {
        return shader;
    }

    const UniformLayout& StandardMaterial::GetUniformLayout() const
    {
        return uniformLayout;
    }

    void StandardMaterial::SetMatrix(const std::string& name, const glm::mat4& matrix)
    {
        if (name == "model")
            SetModelMatrix(matrix);
        else if (name == "view")
            SetViewMatrix(matrix);
        else if (name == "projection")
            SetProjectionMatrix(matrix);
    }

    void StandardMaterial::SetVector(const std::string& name, const glm::vec4& vector)
    {
        (void)name;
        (void)vector;
    }

    void StandardMaterial::SetFloat(const std::string& name, float value)
    {
        (void)name;
        (void)value;
    }

    void StandardMaterial::SetInt(const std::string& name, int32_t value)
    {
        (void)name;
        (void)value;
    }

    void StandardMaterial::UploadUniforms()
    {
        if (!dirty || !uniformBuffer)
            return;

        uniformBuffer->SetData(&mvpMatrices, sizeof(MVPMatrices));
        dirty = false;
    }

    bool StandardMaterial::IsValid() const
    {
        return shader != nullptr && shader->IsValid();
    }

    void StandardMaterial::SetModelMatrix(const glm::mat4& model)
    {
        mvpMatrices.model = model;
        dirty = true;
    }

    void StandardMaterial::SetViewMatrix(const glm::mat4& view)
    {
        mvpMatrices.view = view;
        dirty = true;
    }

    void StandardMaterial::SetProjectionMatrix(const glm::mat4& projection)
    {
        mvpMatrices.projection = projection;
        dirty = true;
    }

    const MVPMatrices& StandardMaterial::GetMVPMatrices() const
    {
        return mvpMatrices;
    }
}
