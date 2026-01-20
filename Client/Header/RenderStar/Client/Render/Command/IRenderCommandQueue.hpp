#pragma once

#include <cstdint>

namespace RenderStar::Client::Render
{
    class IRenderCommandBuffer;

    class IRenderCommandQueue
    {
    public:

        virtual ~IRenderCommandQueue() = default;

        virtual IRenderCommandBuffer* AcquireCommandBuffer() = 0;
        virtual void ReleaseCommandBuffer(IRenderCommandBuffer* buffer) = 0;
        virtual void Submit(IRenderCommandBuffer* buffer) = 0;

        virtual int32_t GetCurrentFrameIndex() const = 0;
    };
}
