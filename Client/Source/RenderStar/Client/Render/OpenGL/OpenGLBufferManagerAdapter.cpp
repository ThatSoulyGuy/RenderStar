#include "RenderStar/Client/Render/OpenGL/OpenGLBufferManagerAdapter.hpp"
#include "RenderStar/Client/Render/OpenGL/OpenGLBufferHandle.hpp"
#include <glad/gl.h>

namespace RenderStar::Client::Render::OpenGL
{
    OpenGLBufferManagerAdapter::OpenGLBufferManagerAdapter()
        : logger(spdlog::default_logger())
    {
    }

    OpenGLBufferManagerAdapter::~OpenGLBufferManagerAdapter()
    {
    }

    std::unique_ptr<IBufferHandle> OpenGLBufferManagerAdapter::CreateBuffer(
        BufferType type,
        BufferUsage usage,
        size_t size,
        const void* initialData)
    {
        GLuint bufferId = 0;
        glGenBuffers(1, &bufferId);

        GLenum target = GL_ARRAY_BUFFER;
        switch (type)
        {
            case BufferType::VERTEX:  target = GL_ARRAY_BUFFER; break;
            case BufferType::INDEX:   target = GL_ELEMENT_ARRAY_BUFFER; break;
            case BufferType::UNIFORM: target = GL_UNIFORM_BUFFER; break;
            case BufferType::STORAGE: target = GL_SHADER_STORAGE_BUFFER; break;
        }

        glBindBuffer(target, bufferId);
        glBufferData(target, static_cast<GLsizeiptr>(size), initialData, ToGLUsage(usage));
        glBindBuffer(target, 0);

        logger->debug("Created buffer {} of type {} with size {}", bufferId, static_cast<int>(type), size);

        return std::make_unique<OpenGLBufferHandle>(bufferId, type, usage, size);
    }

    std::unique_ptr<IBufferHandle> OpenGLBufferManagerAdapter::CreateVertexBuffer(
        size_t size,
        const void* data,
        BufferUsage usage)
    {
        return CreateBuffer(BufferType::VERTEX, usage, size, data);
    }

    std::unique_ptr<IBufferHandle> OpenGLBufferManagerAdapter::CreateIndexBuffer(
        size_t size,
        const void* data,
        BufferUsage usage)
    {
        return CreateBuffer(BufferType::INDEX, usage, size, data);
    }

    std::unique_ptr<IBufferHandle> OpenGLBufferManagerAdapter::CreateUniformBuffer(
        size_t size,
        BufferUsage usage)
    {
        return CreateBuffer(BufferType::UNIFORM, usage, size, nullptr);
    }

    std::unique_ptr<IMesh> OpenGLBufferManagerAdapter::CreateMesh(
        const VertexLayout& layout,
        PrimitiveType primitive)
    {
        (void)layout;
        (void)primitive;
        logger->warn("CreateMesh not implemented for OpenGL adapter - use OpenGLMesh directly");
        return nullptr;
    }

    void OpenGLBufferManagerAdapter::DestroyBuffer(IBufferHandle* buffer)
    {
        if (buffer == nullptr)
            return;

        auto* glBuffer = dynamic_cast<OpenGLBufferHandle*>(buffer);
        if (glBuffer != nullptr && glBuffer->IsValid())
        {
            GLuint bufferId = glBuffer->GetBufferId();
            glDeleteBuffers(1, &bufferId);
            logger->debug("Destroyed buffer {}", bufferId);
        }
    }

    uint32_t OpenGLBufferManagerAdapter::ToGLUsage(BufferUsage usage) const
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
