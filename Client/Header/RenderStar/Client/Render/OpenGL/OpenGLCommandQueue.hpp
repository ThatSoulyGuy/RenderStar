#pragma once

#include "RenderStar/Client/Render/Command/IRenderCommandQueue.hpp"
#include <memory>
#include <vector>
#include <spdlog/spdlog.h>

namespace RenderStar::Client::Render::OpenGL
{
    class OpenGLCommandBuffer;

    class OpenGLCommandQueue : public IRenderCommandQueue
    {
    public:

        OpenGLCommandQueue();
        ~OpenGLCommandQueue() override;

        IRenderCommandBuffer* AcquireCommandBuffer() override;
        void ReleaseCommandBuffer(IRenderCommandBuffer* buffer) override;
        void Submit(IRenderCommandBuffer* buffer) override;

        int32_t GetCurrentFrameIndex() const override;

    private:

        std::shared_ptr<spdlog::logger> logger;
        std::vector<std::unique_ptr<OpenGLCommandBuffer>> commandBufferPool;
        std::vector<OpenGLCommandBuffer*> availableBuffers;
        int32_t frameIndex;
    };
}
