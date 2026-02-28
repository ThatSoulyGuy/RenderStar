#include "RenderStar/Client/Render/OpenGL/OpenGLUniformBinding.hpp"
#include "RenderStar/Client/Render/OpenGL/OpenGLBufferHandle.hpp"
#include "RenderStar/Client/Render/OpenGL/OpenGLTextureHandle.hpp"
#include <glad/gl.h>

namespace RenderStar::Client::Render::OpenGL
{
    OpenGLUniformBinding::OpenGLUniformBinding(const UniformLayout& layout, int32_t frameCount)
        : logger(spdlog::default_logger())
        , layout(layout)
        , frameCount(frameCount)
        , destroyed(false)
    {
    }

    OpenGLUniformBinding::~OpenGLUniformBinding()
    {
        if (!released)
            Release();
    }

    void OpenGLUniformBinding::Release()
    {
        if (released)
            return;

        Destroy();
        released = true;
    }

    GraphicsResourceType OpenGLUniformBinding::GetResourceType() const
    {
        return GraphicsResourceType::UNIFORM_BINDING;
    }

    void OpenGLUniformBinding::Bind(int32_t frameIndex)
    {
        if (destroyed)
            return;

        for (const auto& [bindingPoint, buffers] : bindingBuffers)
        {
            if (frameIndex < static_cast<int32_t>(buffers.size()) && buffers[frameIndex] != nullptr)
            {
                glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, buffers[frameIndex]->GetBufferId());
            }
        }

        for (const auto& [unit, texture] : textureBindings)
        {
            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, texture->GetTextureId());
        }
    }

    void OpenGLUniformBinding::UpdateBuffer(int32_t binding, IBufferHandle* buffer, size_t size, int32_t frameIndex)
    {
        if (destroyed)
            return;

        auto* glBuffer = static_cast<OpenGLBufferHandle*>(buffer);
        if (glBuffer == nullptr)
            return;

        auto& buffers = bindingBuffers[binding];
        buffers.resize(frameCount, nullptr);

        for (int32_t i = 0; i < frameCount; ++i)
            buffers[i] = glBuffer;
    }

    void OpenGLUniformBinding::UpdateTexture(int32_t binding, ITextureHandle* texture, int32_t frameIndex)
    {
        if (destroyed)
            return;

        if (texture == nullptr)
        {
            textureBindings.erase(binding);
            return;
        }

        textureBindings[binding] = static_cast<OpenGLTextureHandle*>(texture);
    }

    uint64_t OpenGLUniformBinding::GetNativeHandle(int32_t frameIndex) const
    {
        return 0;
    }

    void OpenGLUniformBinding::Destroy()
    {
        if (destroyed)
            return;

        bindingBuffers.clear();
        textureBindings.clear();
        destroyed = true;

        logger->debug("OpenGL uniform binding destroyed");
    }

    bool OpenGLUniformBinding::IsDestroyed() const
    {
        return destroyed;
    }

    void OpenGLUniformBinding::UpdateBufferForFrame(int32_t binding, int32_t frameIndex, OpenGLBufferHandle* buffer)
    {
        if (destroyed)
            return;

        auto& buffers = bindingBuffers[binding];
        buffers.resize(frameCount, nullptr);

        if (frameIndex < static_cast<int32_t>(buffers.size()))
            buffers[frameIndex] = buffer;
    }

    const UniformLayout& OpenGLUniformBinding::GetLayout() const
    {
        return layout;
    }
}
