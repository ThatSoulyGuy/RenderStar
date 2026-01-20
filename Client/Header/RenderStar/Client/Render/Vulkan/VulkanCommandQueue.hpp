#pragma once

#include "RenderStar/Client/Render/Command/IRenderCommandQueue.hpp"
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <spdlog/spdlog.h>

namespace RenderStar::Client::Render::Vulkan
{
    class VulkanCommandModule;
    class VulkanCommandBufferAdapter;

    class VulkanCommandQueue : public IRenderCommandQueue
    {
    public:

        VulkanCommandQueue();
        ~VulkanCommandQueue() override;

        void Initialize(VulkanCommandModule* commandModule, int32_t maxFramesInFlight);

        IRenderCommandBuffer* AcquireCommandBuffer() override;
        void ReleaseCommandBuffer(IRenderCommandBuffer* buffer) override;
        void Submit(IRenderCommandBuffer* buffer) override;

        int32_t GetCurrentFrameIndex() const override;
        void SetCurrentFrameIndex(int32_t index);

    private:

        std::shared_ptr<spdlog::logger> logger;
        VulkanCommandModule* commandModule;
        std::vector<std::unique_ptr<VulkanCommandBufferAdapter>> adapters;
        int32_t currentFrameIndex;
        int32_t maxFramesInFlight;
    };
}
