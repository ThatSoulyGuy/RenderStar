#include "RenderStar/Client/Render/OpenGL/OpenGLMesh.hpp"
#include <glad/gl.h>

namespace RenderStar::Client::Render::OpenGL
{
    OpenGLMesh::OpenGLMesh()
        : logger(spdlog::default_logger())
        , vaoHandle(INVALID_VAO)
        , vertexBuffer(0)
        , indexBuffer(0)
        , attributes()
    {
    }

    OpenGLMesh::~OpenGLMesh()
    {
        Destroy();
    }

    void OpenGLMesh::Create(const std::vector<VertexAttribute>& vertexAttributes)
    {
        attributes = vertexAttributes;

        glGenVertexArrays(1, &vaoHandle);
        glBindVertexArray(vaoHandle);

        for (const auto& attribute : attributes)
        {
            glEnableVertexAttribArray(attribute.location);
            glVertexAttribPointer(
                attribute.location,
                attribute.componentCount,
                attribute.type,
                attribute.normalized ? GL_TRUE : GL_FALSE,
                attribute.stride,
                reinterpret_cast<const void*>(attribute.offset)
            );
        }

        glBindVertexArray(0);

        logger->debug("Created VAO {} with {} attributes", vaoHandle, attributes.size());
    }

    void OpenGLMesh::Destroy()
    {
        if (vaoHandle != INVALID_VAO)
        {
            glDeleteVertexArrays(1, &vaoHandle);
            logger->debug("Destroyed VAO {}", vaoHandle);
            vaoHandle = INVALID_VAO;
        }

        vertexBuffer = 0;
        indexBuffer = 0;
    }

    void OpenGLMesh::Bind()
    {
        if (vaoHandle != INVALID_VAO)
            glBindVertexArray(vaoHandle);
    }

    void OpenGLMesh::Unbind()
    {
        glBindVertexArray(0);
    }

    void OpenGLMesh::SetVertexBuffer(uint32_t buffer)
    {
        vertexBuffer = buffer;

        if (vaoHandle != INVALID_VAO)
        {
            glBindVertexArray(vaoHandle);
            glBindBuffer(GL_ARRAY_BUFFER, buffer);

            for (const auto& attribute : attributes)
            {
                glEnableVertexAttribArray(attribute.location);
                glVertexAttribPointer(
                    attribute.location,
                    attribute.componentCount,
                    attribute.type,
                    attribute.normalized ? GL_TRUE : GL_FALSE,
                    attribute.stride,
                    reinterpret_cast<const void*>(attribute.offset)
                );
            }

            glBindVertexArray(0);
        }
    }

    void OpenGLMesh::SetIndexBuffer(uint32_t buffer)
    {
        indexBuffer = buffer;

        if (vaoHandle != INVALID_VAO)
        {
            glBindVertexArray(vaoHandle);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
            glBindVertexArray(0);
        }
    }

    void OpenGLMesh::Draw(int32_t indexCount)
    {
        if (vaoHandle == INVALID_VAO)
            return;

        glBindVertexArray(vaoHandle);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }

    void OpenGLMesh::DrawInstanced(int32_t indexCount, int32_t instanceCount)
    {
        if (vaoHandle == INVALID_VAO)
            return;

        glBindVertexArray(vaoHandle);
        glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr, instanceCount);
        glBindVertexArray(0);
    }

    VertexArrayHandle OpenGLMesh::GetHandle() const
    {
        return vaoHandle;
    }

    bool OpenGLMesh::IsValid() const
    {
        return vaoHandle != INVALID_VAO;
    }
}
