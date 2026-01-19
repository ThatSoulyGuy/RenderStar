#pragma once

#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>
#include <vector>

namespace RenderStar::Client::Render::Vulkan
{
    class VulkanFramebufferModule
    {
    public:

        VulkanFramebufferModule();

        ~VulkanFramebufferModule();

        void Create(
            VkDevice device,
            const std::vector<VkImageView>& imageViews,
            VkImageView depthImageView,
            VkRenderPass renderPass,
            int32_t width,
            int32_t height
        );

        void Destroy(VkDevice device);

        const std::vector<VkFramebuffer>& GetFramebuffers() const;

        VkFramebuffer GetFramebuffer(uint32_t index) const;

    private:

        std::shared_ptr<spdlog::logger> logger;
        std::vector<VkFramebuffer> framebuffers;
    };
}
