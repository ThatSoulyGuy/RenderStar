#pragma once

#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>
#include <vector>

namespace RenderStar::Client::Render::Vulkan
{
    class VulkanSyncModule
    {
    public:

        static constexpr int32_t MAX_FRAMES_IN_FLIGHT = 2;

        VulkanSyncModule();

        ~VulkanSyncModule();

        void Create(VkDevice device);

        void Destroy(VkDevice device);

        VkSemaphore GetImageAvailableSemaphore(int32_t frame) const;

        VkSemaphore GetRenderFinishedSemaphore(int32_t frame) const;

        VkFence GetInFlightFence(int32_t frame) const;

    private:

        std::shared_ptr<spdlog::logger> logger;
        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;
    };
}
