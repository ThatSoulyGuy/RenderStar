#include "RenderStar/Client/Render/OpenGL/OpenGLCommandQueue.hpp"
#include "RenderStar/Client/Render/OpenGL/OpenGLCommandBuffer.hpp"
#include <algorithm>

namespace RenderStar::Client::Render::OpenGL
{
    OpenGLCommandQueue::OpenGLCommandQueue()
        : logger(spdlog::default_logger())
        , frameIndex(0)
    {
    }

    OpenGLCommandQueue::~OpenGLCommandQueue()
    {
        commandBufferPool.clear();
        availableBuffers.clear();
    }

    IRenderCommandBuffer* OpenGLCommandQueue::AcquireCommandBuffer()
    {
        if (!availableBuffers.empty())
        {
            OpenGLCommandBuffer* buffer = availableBuffers.back();
            availableBuffers.pop_back();
            buffer->Reset();
            return buffer;
        }

        auto newBuffer = std::make_unique<OpenGLCommandBuffer>();
        OpenGLCommandBuffer* rawPtr = newBuffer.get();
        commandBufferPool.push_back(std::move(newBuffer));
        return rawPtr;
    }

    void OpenGLCommandQueue::ReleaseCommandBuffer(IRenderCommandBuffer* buffer)
    {
        auto* glBuffer = static_cast<OpenGLCommandBuffer*>(buffer);
        if (glBuffer != nullptr)
        {
            glBuffer->Reset();
            availableBuffers.push_back(glBuffer);
        }
    }

    void OpenGLCommandQueue::Submit(IRenderCommandBuffer* buffer)
    {
        ReleaseCommandBuffer(buffer);
    }

    int32_t OpenGLCommandQueue::GetCurrentFrameIndex() const
    {
        return frameIndex;
    }
}
