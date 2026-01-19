#pragma once

#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>
#include <vector>

namespace RenderStar::Client::Render::Vulkan
{
    class VulkanCommandModule
    {
    public:

        VulkanCommandModule();

        ~VulkanCommandModule();

        void CreateCommandPool(VkDevice device, uint32_t graphicsQueueFamily);

        void AllocateCommandBuffers(VkDevice device, uint32_t count);

        void BeginRecording(VkCommandBuffer commandBuffer);

        void BeginRenderPass(
            VkRenderPass renderPass,
            VkFramebuffer framebuffer,
            int32_t width,
            int32_t height
        );

        void EndRenderPass();

        void EndRecording();

        void Destroy(VkDevice device);

        VkCommandPool GetCommandPool() const;

        const std::vector<VkCommandBuffer>& GetCommandBuffers() const;

        VkCommandBuffer GetCommandBuffer(uint32_t index) const;

        VkCommandBuffer GetCurrentCommandBuffer() const;

    private:

        std::shared_ptr<spdlog::logger> logger;
        VkCommandPool commandPool;
        std::vector<VkCommandBuffer> commandBuffers;
        VkCommandBuffer currentCommandBuffer;
    };
}
