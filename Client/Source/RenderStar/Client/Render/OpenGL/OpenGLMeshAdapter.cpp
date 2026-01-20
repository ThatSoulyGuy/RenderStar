#include "RenderStar/Client/Render/OpenGL/OpenGLMeshAdapter.hpp"
#include <glad/gl.h>

namespace RenderStar::Client::Render::OpenGL
{
    OpenGLMeshAdapter::OpenGLMeshAdapter(const VertexLayout& layout, PrimitiveType primitive)
        : logger(spdlog::default_logger())
        , layout(layout)
        , primitive(primitive)
        , vaoHandle(0)
        , vertexBuffer(0)
        , indexBuffer(0)
        , vertexCount(0)
        , indexCount(0)
        , currentIndexType(IndexType::UINT32)
        , hasIndices(false)
        , valid(false)
    {
        glGenVertexArrays(1, &vaoHandle);
        glGenBuffers(1, &vertexBuffer);
        valid = (vaoHandle != 0 && vertexBuffer != 0);
    }

    OpenGLMeshAdapter::~OpenGLMeshAdapter()
    {
        if (indexBuffer != 0)
            glDeleteBuffers(1, &indexBuffer);

        if (vertexBuffer != 0)
            glDeleteBuffers(1, &vertexBuffer);

        if (vaoHandle != 0)
            glDeleteVertexArrays(1, &vaoHandle);
    }

    void OpenGLMeshAdapter::Bind()
    {
        if (vaoHandle != 0)
            glBindVertexArray(vaoHandle);
    }

    void OpenGLMeshAdapter::Unbind()
    {
        glBindVertexArray(0);
    }

    void OpenGLMeshAdapter::Draw()
    {
        if (!valid)
            return;

        Bind();

        if (hasIndices && indexCount > 0)
        {
            GLenum indexTypeGL = (currentIndexType == IndexType::UINT16) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
            glDrawElements(ToGLPrimitive(primitive), indexCount, indexTypeGL, nullptr);
        }
        else if (vertexCount > 0)
        {
            glDrawArrays(ToGLPrimitive(primitive), 0, vertexCount);
        }

        Unbind();
    }

    void OpenGLMeshAdapter::DrawInstanced(int32_t instanceCount)
    {
        if (!valid || instanceCount <= 0)
            return;

        Bind();

        if (hasIndices && indexCount > 0)
        {
            GLenum indexTypeGL = (currentIndexType == IndexType::UINT16) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
            glDrawElementsInstanced(ToGLPrimitive(primitive), indexCount, indexTypeGL, nullptr, instanceCount);
        }
        else if (vertexCount > 0)
        {
            glDrawArraysInstanced(ToGLPrimitive(primitive), 0, vertexCount, instanceCount);
        }

        Unbind();
    }

    void OpenGLMeshAdapter::SetVertexData(const void* data, size_t size)
    {
        if (!valid || data == nullptr || size == 0)
            return;

        glBindVertexArray(vaoHandle);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(size), data, GL_STATIC_DRAW);

        SetupVertexAttributes();

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        vertexCount = static_cast<int32_t>(size / layout.stride);
    }

    void OpenGLMeshAdapter::SetIndexData(const void* data, size_t size, IndexType indexType)
    {
        if (!valid || data == nullptr || size == 0)
            return;

        if (indexBuffer == 0)
            glGenBuffers(1, &indexBuffer);

        glBindVertexArray(vaoHandle);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(size), data, GL_STATIC_DRAW);
        glBindVertexArray(0);

        currentIndexType = indexType;
        size_t indexSize = (indexType == IndexType::UINT16) ? sizeof(uint16_t) : sizeof(uint32_t);
        indexCount = static_cast<int32_t>(size / indexSize);
        hasIndices = true;
    }

    int32_t OpenGLMeshAdapter::GetVertexCount() const
    {
        return vertexCount;
    }

    int32_t OpenGLMeshAdapter::GetIndexCount() const
    {
        return indexCount;
    }

    const VertexLayout& OpenGLMeshAdapter::GetVertexLayout() const
    {
        return layout;
    }

    PrimitiveType OpenGLMeshAdapter::GetPrimitiveType() const
    {
        return primitive;
    }

    bool OpenGLMeshAdapter::HasIndices() const
    {
        return hasIndices;
    }

    bool OpenGLMeshAdapter::IsValid() const
    {
        return valid;
    }

    void OpenGLMeshAdapter::SetupVertexAttributes()
    {
        for (const auto& attr : layout.attributes)
        {
            glEnableVertexAttribArray(attr.location);
            glVertexAttribPointer(
                attr.location,
                VertexAttribute::GetComponentCount(attr.type),
                ToGLType(attr.type),
                attr.normalized ? GL_TRUE : GL_FALSE,
                layout.stride,
                reinterpret_cast<const void*>(attr.offset)
            );
        }
    }

    uint32_t OpenGLMeshAdapter::ToGLPrimitive(PrimitiveType type) const
    {
        switch (type)
        {
            case PrimitiveType::TRIANGLES:      return GL_TRIANGLES;
            case PrimitiveType::TRIANGLE_STRIP: return GL_TRIANGLE_STRIP;
            case PrimitiveType::TRIANGLE_FAN:   return GL_TRIANGLE_FAN;
            case PrimitiveType::LINES:          return GL_LINES;
            case PrimitiveType::LINE_STRIP:     return GL_LINE_STRIP;
            case PrimitiveType::POINTS:         return GL_POINTS;
        }
        return GL_TRIANGLES;
    }

    uint32_t OpenGLMeshAdapter::ToGLType(VertexAttributeType type) const
    {
        switch (type)
        {
            case VertexAttributeType::FLOAT:
            case VertexAttributeType::FLOAT2:
            case VertexAttributeType::FLOAT3:
            case VertexAttributeType::FLOAT4:
                return GL_FLOAT;
            case VertexAttributeType::INT:
            case VertexAttributeType::INT2:
            case VertexAttributeType::INT3:
            case VertexAttributeType::INT4:
                return GL_INT;
            case VertexAttributeType::UINT:
            case VertexAttributeType::UINT2:
            case VertexAttributeType::UINT3:
            case VertexAttributeType::UINT4:
                return GL_UNSIGNED_INT;
        }
        return GL_FLOAT;
    }
}
