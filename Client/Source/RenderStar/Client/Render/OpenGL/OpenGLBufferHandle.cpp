#include "RenderStar/Client/Render/OpenGL/OpenGLBufferHandle.hpp"
#include <glad/gl.h>

namespace RenderStar::Client::Render::OpenGL
{
    namespace
    {
        GLenum GetGLBufferTarget(BufferType type)
        {
            switch (type)
            {
                case BufferType::VERTEX:  return GL_ARRAY_BUFFER;
                case BufferType::INDEX:   return GL_ELEMENT_ARRAY_BUFFER;
                case BufferType::UNIFORM: return GL_UNIFORM_BUFFER;
                case BufferType::STORAGE: return GL_SHADER_STORAGE_BUFFER;
            }
            return GL_ARRAY_BUFFER;
        }

        GLenum GetGLBufferUsage(BufferUsage usage)
        {
            switch (usage)
            {
                case BufferUsage::STATIC:  return GL_STATIC_DRAW;
                case BufferUsage::DYNAMIC: return GL_DYNAMIC_DRAW;
                case BufferUsage::STREAM:  return GL_STREAM_DRAW;
            }
            return GL_STATIC_DRAW;
        }
    }

    OpenGLBufferHandle::OpenGLBufferHandle(uint32_t bufferId, BufferType type, BufferUsage usage, size_t size)
        : bufferId(bufferId)
        , type(type)
        , usage(usage)
        , size(size)
        , destroyed(false)
    {
    }

    OpenGLBufferHandle::~OpenGLBufferHandle()
    {
        if (!destroyed && bufferId != 0)
        {
            glDeleteBuffers(1, &bufferId);
            destroyed = true;
        }
    }

    void OpenGLBufferHandle::Bind()
    {
        if (!destroyed && bufferId != 0)
            glBindBuffer(GetGLBufferTarget(type), bufferId);
    }

    void OpenGLBufferHandle::Unbind()
    {
        glBindBuffer(GetGLBufferTarget(type), 0);
    }

    void OpenGLBufferHandle::SetData(const void* data, size_t dataSize)
    {
        if (destroyed || bufferId == 0)
            return;

        glBindBuffer(GetGLBufferTarget(type), bufferId);
        glBufferData(GetGLBufferTarget(type), dataSize, data, GetGLBufferUsage(usage));
        glBindBuffer(GetGLBufferTarget(type), 0);
        size = dataSize;
    }

    void OpenGLBufferHandle::SetSubData(const void* data, size_t dataSize, size_t offset)
    {
        if (destroyed || bufferId == 0)
            return;

        glBindBuffer(GetGLBufferTarget(type), bufferId);
        glBufferSubData(GetGLBufferTarget(type), offset, dataSize, data);
        glBindBuffer(GetGLBufferTarget(type), 0);
    }

    size_t OpenGLBufferHandle::GetSize() const
    {
        return size;
    }

    BufferType OpenGLBufferHandle::GetType() const
    {
        return type;
    }

    BufferUsage OpenGLBufferHandle::GetUsage() const
    {
        return usage;
    }

    bool OpenGLBufferHandle::IsValid() const
    {
        return !destroyed && bufferId != 0;
    }

    uint32_t OpenGLBufferHandle::GetBufferId() const
    {
        return bufferId;
    }

    bool OpenGLBufferHandle::IsDestroyed() const
    {
        return destroyed;
    }
}
