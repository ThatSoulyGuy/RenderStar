#include "RenderStar/Client/Render/Vulkan/VulkanSyncModule.hpp"

namespace RenderStar::Client::Render::Vulkan
{
    VulkanSyncModule::VulkanSyncModule()
        : logger(spdlog::default_logger())
    {
    }

    VulkanSyncModule::~VulkanSyncModule()
    {
    }

    void VulkanSyncModule::Create(VkDevice device)
    {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (int32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            VkResult result = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]);
            if (result != VK_SUCCESS)
            {
                logger->error("Failed to create image available semaphore: {}", static_cast<int>(result));
                return;
            }

            result = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]);
            if (result != VK_SUCCESS)
            {
                logger->error("Failed to create render finished semaphore: {}", static_cast<int>(result));
                return;
            }

            result = vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]);
            if (result != VK_SUCCESS)
            {
                logger->error("Failed to create fence: {}", static_cast<int>(result));
                return;
            }
        }

        logger->info("Synchronization objects created");
    }

    void VulkanSyncModule::Destroy(VkDevice device)
    {
        for (int32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }

        imageAvailableSemaphores.clear();
        renderFinishedSemaphores.clear();
        inFlightFences.clear();

        logger->info("Synchronization objects destroyed");
    }

    VkSemaphore VulkanSyncModule::GetImageAvailableSemaphore(int32_t frame) const
    {
        return imageAvailableSemaphores[frame];
    }

    VkSemaphore VulkanSyncModule::GetRenderFinishedSemaphore(int32_t frame) const
    {
        return renderFinishedSemaphores[frame];
    }

    VkFence VulkanSyncModule::GetInFlightFence(int32_t frame) const
    {
        return inFlightFences[frame];
    }
}
