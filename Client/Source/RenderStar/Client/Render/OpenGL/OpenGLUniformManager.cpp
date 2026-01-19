#include "RenderStar/Client/Render/OpenGL/OpenGLUniformManager.hpp"
#include <glad/gl.h>

namespace RenderStar::Client::Render::OpenGL
{
    OpenGLUniformManager::OpenGLUniformManager()
        : logger(spdlog::default_logger())
    {
    }

    OpenGLUniformManager::~OpenGLUniformManager()
    {
        for (auto& [name, info] : uniformBlocks)
        {
            if (info.bufferHandle != 0)
            {
                glDeleteBuffers(1, &info.bufferHandle);
            }
        }

        uniformBlocks.clear();
    }

    void OpenGLUniformManager::RegisterUniformBlock(const std::string& blockName, uint32_t bindingPoint, size_t bufferSize)
    {
        if (uniformBlocks.contains(blockName))
        {
            logger->warn("Uniform block '{}' already registered", blockName);
            return;
        }

        GLuint ubo = 0;
        glGenBuffers(1, &ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, ubo);
        glBufferData(GL_UNIFORM_BUFFER, static_cast<GLsizeiptr>(bufferSize), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, ubo);

        UniformBlockInfo info;
        info.name = blockName;
        info.bindingPoint = bindingPoint;
        info.bufferHandle = ubo;
        info.bufferSize = bufferSize;

        uniformBlocks[blockName] = info;

        logger->debug("Registered uniform block '{}' at binding point {} with buffer {} (size {})",
            blockName, bindingPoint, ubo, bufferSize);
    }

    void OpenGLUniformManager::UpdateUniformBlock(const std::string& blockName, const void* data, size_t size, size_t offset)
    {
        auto iterator = uniformBlocks.find(blockName);

        if (iterator == uniformBlocks.end())
        {
            logger->warn("Uniform block '{}' not found", blockName);
            return;
        }

        const auto& info = iterator->second;

        if (offset + size > info.bufferSize)
        {
            logger->error("Update exceeds buffer size for uniform block '{}'", blockName);
            return;
        }

        glBindBuffer(GL_UNIFORM_BUFFER, info.bufferHandle);
        glBufferSubData(GL_UNIFORM_BUFFER, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), data);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void OpenGLUniformManager::BindUniformBlock(const std::string& blockName)
    {
        auto iterator = uniformBlocks.find(blockName);

        if (iterator == uniformBlocks.end())
        {
            logger->warn("Uniform block '{}' not found", blockName);
            return;
        }

        const auto& info = iterator->second;
        glBindBufferBase(GL_UNIFORM_BUFFER, info.bindingPoint, info.bufferHandle);
    }

    void OpenGLUniformManager::UnbindUniformBlock(const std::string& blockName)
    {
        auto iterator = uniformBlocks.find(blockName);

        if (iterator == uniformBlocks.end())
        {
            logger->warn("Uniform block '{}' not found", blockName);
            return;
        }

        const auto& info = iterator->second;
        glBindBufferBase(GL_UNIFORM_BUFFER, info.bindingPoint, 0);
    }

    bool OpenGLUniformManager::HasUniformBlock(const std::string& blockName) const
    {
        return uniformBlocks.contains(blockName);
    }

    uint32_t OpenGLUniformManager::GetBindingPoint(const std::string& blockName) const
    {
        auto iterator = uniformBlocks.find(blockName);

        if (iterator == uniformBlocks.end())
            return 0;

        return iterator->second.bindingPoint;
    }
}
