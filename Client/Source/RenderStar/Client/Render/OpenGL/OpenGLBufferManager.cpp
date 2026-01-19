#include "RenderStar/Client/Render/OpenGL/OpenGLBufferManager.hpp"
#include <glad/gl.h>
#include <algorithm>

namespace RenderStar::Client::Render::OpenGL
{
    OpenGLBufferManager::OpenGLBufferManager()
        : logger(spdlog::default_logger())
    {
    }

    OpenGLBufferManager::~OpenGLBufferManager()
    {
        for (BufferHandle handle : allocatedBuffers)
        {
            if (handle != INVALID_BUFFER)
                glDeleteBuffers(1, &handle);
        }

        allocatedBuffers.clear();
    }

    BufferHandle OpenGLBufferManager::CreateVertexBuffer(const void* data, size_t size)
    {
        GLuint vbo = 0;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(size), data, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        allocatedBuffers.push_back(vbo);
        logger->debug("Created vertex buffer {} with size {}", vbo, size);

        return vbo;
    }

    BufferHandle OpenGLBufferManager::CreateIndexBuffer(const void* data, size_t size)
    {
        GLuint ebo = 0;
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(size), data, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        allocatedBuffers.push_back(ebo);
        logger->debug("Created index buffer {} with size {}", ebo, size);

        return ebo;
    }

    BufferHandle OpenGLBufferManager::CreateUniformBuffer(size_t size)
    {
        GLuint ubo = 0;
        glGenBuffers(1, &ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, ubo);
        glBufferData(GL_UNIFORM_BUFFER, static_cast<GLsizeiptr>(size), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        allocatedBuffers.push_back(ubo);
        logger->debug("Created uniform buffer {} with size {}", ubo, size);

        return ubo;
    }

    void OpenGLBufferManager::UpdateBuffer(BufferHandle handle, const void* data, size_t size, size_t offset)
    {
        if (handle == INVALID_BUFFER)
            return;

        glBindBuffer(GL_UNIFORM_BUFFER, handle);
        glBufferSubData(GL_UNIFORM_BUFFER, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), data);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void OpenGLBufferManager::DestroyBuffer(BufferHandle handle)
    {
        if (handle == INVALID_BUFFER)
            return;

        auto iterator = std::find(allocatedBuffers.begin(), allocatedBuffers.end(), handle);

        if (iterator != allocatedBuffers.end())
        {
            glDeleteBuffers(1, &handle);
            allocatedBuffers.erase(iterator);
            logger->debug("Destroyed buffer {}", handle);
        }
    }

    void OpenGLBufferManager::BindVertexBuffer(BufferHandle handle)
    {
        glBindBuffer(GL_ARRAY_BUFFER, handle);
    }

    void OpenGLBufferManager::BindIndexBuffer(BufferHandle handle)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
    }

    void OpenGLBufferManager::BindUniformBuffer(BufferHandle handle, uint32_t bindingPoint)
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, handle);
    }
}
