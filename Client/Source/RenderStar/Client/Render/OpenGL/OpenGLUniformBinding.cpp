#include "RenderStar/Client/Render/OpenGL/OpenGLUniformBinding.hpp"
#include "RenderStar/Client/Render/OpenGL/OpenGLBufferHandle.hpp"
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
        Destroy();
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
    }

    void OpenGLUniformBinding::UpdateBuffer(int32_t binding, IBufferHandle* buffer, size_t size)
    {
        if (destroyed)
            return;

        auto* glBuffer = dynamic_cast<OpenGLBufferHandle*>(buffer);
        if (glBuffer == nullptr)
            return;

        auto& buffers = bindingBuffers[binding];
        buffers.resize(frameCount, nullptr);

        for (int32_t i = 0; i < frameCount; ++i)
            buffers[i] = glBuffer;
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
