#include "RenderStar/Client/Render/OpenGL/OpenGLCommandBuffer.hpp"
#include "RenderStar/Client/Render/OpenGL/OpenGLShaderProgram.hpp"
#include "RenderStar/Client/Render/OpenGL/OpenGLBufferHandle.hpp"
#include "RenderStar/Client/Render/OpenGL/OpenGLMeshAdapter.hpp"
#include "RenderStar/Client/Render/OpenGL/OpenGLUniformBinding.hpp"
#include "RenderStar/Client/Render/Resource/IMesh.hpp"
#include <glad/gl.h>

namespace RenderStar::Client::Render::OpenGL
{
    OpenGLCommandBuffer::OpenGLCommandBuffer()
        : logger(spdlog::default_logger())
        , recording(false)
        , currentShader(nullptr)
        , currentVao(0)
        , currentIndexType(IndexType::UINT32)
    {
    }

    OpenGLCommandBuffer::~OpenGLCommandBuffer()
    {
    }

    void OpenGLCommandBuffer::Begin()
    {
        recording = true;
        currentShader = nullptr;
        currentVao = 0;
    }

    void OpenGLCommandBuffer::End()
    {
        if (currentShader != nullptr)
            currentShader->Unbind();

        if (currentVao != 0)
            glBindVertexArray(0);

        recording = false;
    }

    void OpenGLCommandBuffer::BindPipeline(IShaderProgram* shader)
    {
        if (!recording)
            return;

        auto* glShader = static_cast<OpenGLShaderProgram*>(shader);
        if (glShader != nullptr)
        {
            glShader->Bind();
            currentShader = glShader;
        }
    }

    void OpenGLCommandBuffer::BindVertexBuffer(IBufferHandle* buffer, uint32_t binding)
    {
        if (!recording)
            return;

        auto* glBuffer = static_cast<OpenGLBufferHandle*>(buffer);
        if (glBuffer != nullptr)
            glBindBuffer(GL_ARRAY_BUFFER, glBuffer->GetBufferId());
    }

    void OpenGLCommandBuffer::BindIndexBuffer(IBufferHandle* buffer, IndexType type)
    {
        if (!recording)
            return;

        auto* glBuffer = static_cast<OpenGLBufferHandle*>(buffer);
        if (glBuffer != nullptr)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBuffer->GetBufferId());
            currentIndexType = type;
        }
    }

    void OpenGLCommandBuffer::BindUniformSet(IUniformBindingHandle* uniforms, uint32_t set)
    {
        if (!recording)
            return;

        auto* glBinding = static_cast<OpenGLUniformBinding*>(uniforms);
        if (glBinding != nullptr)
            glBinding->Bind(0);
    }

    void OpenGLCommandBuffer::Draw(uint32_t vertexCount, uint32_t firstVertex)
    {
        if (!recording)
            return;

        glDrawArrays(GL_TRIANGLES, firstVertex, vertexCount);
    }

    void OpenGLCommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t firstIndex)
    {
        if (!recording)
            return;

        GLenum indexTypeGL = (currentIndexType == IndexType::UINT16) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
        size_t offset = (currentIndexType == IndexType::UINT16) ? firstIndex * sizeof(uint16_t) : firstIndex * sizeof(uint32_t);
        glDrawElements(GL_TRIANGLES, indexCount, indexTypeGL, reinterpret_cast<const void*>(offset));
    }

    void OpenGLCommandBuffer::DrawMesh(IMesh* mesh)
    {
        if (!recording || mesh == nullptr)
            return;

        auto* glMesh = static_cast<OpenGLMeshAdapter*>(mesh);
        if (glMesh != nullptr && glMesh->IsValid())
        {
            glMesh->Bind();
            glMesh->Draw();
            glMesh->Unbind();
        }
    }

    void OpenGLCommandBuffer::SetViewport(float x, float y, float width, float height)
    {
        if (!recording)
            return;

        glViewport(static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(width), static_cast<GLsizei>(height));
    }

    void OpenGLCommandBuffer::SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height)
    {
        if (!recording)
            return;

        glScissor(x, y, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
    }

    void OpenGLCommandBuffer::Reset()
    {
        recording = false;
        currentShader = nullptr;
        currentVao = 0;
    }

    bool OpenGLCommandBuffer::IsRecording() const
    {
        return recording;
    }
}
