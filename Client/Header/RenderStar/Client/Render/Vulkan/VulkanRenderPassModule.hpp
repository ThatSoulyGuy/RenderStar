#pragma once

#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>

namespace RenderStar::Client::Render::Vulkan
{
    class VulkanRenderPassModule
    {
    public:

        VulkanRenderPassModule();

        ~VulkanRenderPassModule();

        void Create(VkDevice device, VkFormat imageFormat, VkFormat depthFormat);

        void Destroy(VkDevice device);

        VkRenderPass GetRenderPass() const;

    private:

        std::shared_ptr<spdlog::logger> logger;
        VkRenderPass renderPass;
    };
}
