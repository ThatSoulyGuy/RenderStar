#include "RenderStar/Client/Render/OpenGL/OpenGLUniformManagerAdapter.hpp"
#include "RenderStar/Client/Render/OpenGL/OpenGLBufferHandle.hpp"
#include "RenderStar/Client/Render/OpenGL/OpenGLUniformBinding.hpp"
#include <glad/gl.h>

namespace RenderStar::Client::Render::OpenGL
{
    OpenGLUniformManagerAdapter::OpenGLUniformManagerAdapter()
        : logger(spdlog::default_logger())
        , bufferManager(std::make_unique<OpenGLBufferManagerAdapter>())
    {
    }

    OpenGLUniformManagerAdapter::~OpenGLUniformManagerAdapter()
    {
        uniformBuffers.clear();
    }

    void OpenGLUniformManagerAdapter::CreateUniformBuffer(
        const std::string& name,
        const UniformBinding& binding)
    {
        auto buffer = bufferManager->CreateUniformBuffer(binding.size, BufferUsage::DYNAMIC);
        bufferBindings[name] = binding.binding;
        uniformBuffers[name] = std::move(buffer);
        logger->debug("Created uniform buffer '{}' with binding {} and size {}", name, binding.binding, binding.size);
    }

    void OpenGLUniformManagerAdapter::UpdateUniformBuffer(
        const std::string& name,
        const void* data,
        size_t size,
        size_t offset)
    {
        auto it = uniformBuffers.find(name);
        if (it == uniformBuffers.end())
        {
            logger->warn("Uniform buffer '{}' not found for update", name);
            return;
        }

        it->second->SetSubData(data, size, offset);
    }

    void OpenGLUniformManagerAdapter::BindUniformBuffer(const std::string& name, uint32_t bindingPoint)
    {
        auto it = uniformBuffers.find(name);
        if (it == uniformBuffers.end())
        {
            logger->warn("Uniform buffer '{}' not found for binding", name);
            return;
        }

        auto* glBuffer = static_cast<OpenGLBufferHandle*>(it->second.get());
        if (glBuffer != nullptr)
        {
            glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, glBuffer->GetBufferId());
            bufferBindings[name] = bindingPoint;
        }
    }

    IBufferHandle* OpenGLUniformManagerAdapter::GetUniformBuffer(const std::string& name)
    {
        auto it = uniformBuffers.find(name);
        if (it == uniformBuffers.end())
            return nullptr;

        return it->second.get();
    }

    void OpenGLUniformManagerAdapter::DestroyUniformBuffer(const std::string& name)
    {
        auto it = uniformBuffers.find(name);
        if (it != uniformBuffers.end())
        {
            bufferManager->DestroyBuffer(it->second.get());
            uniformBuffers.erase(it);
            bufferBindings.erase(name);
            logger->debug("Destroyed uniform buffer '{}'", name);
        }
    }

    std::unique_ptr<IUniformBindingHandle> OpenGLUniformManagerAdapter::CreateBindingForShader(IShaderProgram* shader)
    {
        (void)shader;
        UniformLayout layout = UniformLayout::ForMVP();
        return std::make_unique<OpenGLUniformBinding>(layout, 2);
    }
}
