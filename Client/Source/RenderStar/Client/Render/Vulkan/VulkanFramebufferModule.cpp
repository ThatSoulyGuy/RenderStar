#include "RenderStar/Client/Render/Vulkan/VulkanFramebufferModule.hpp"
#include <array>

namespace RenderStar::Client::Render::Vulkan
{
    VulkanFramebufferModule::VulkanFramebufferModule()
        : logger(spdlog::default_logger())
    {
    }

    VulkanFramebufferModule::~VulkanFramebufferModule()
    {
    }

    void VulkanFramebufferModule::Create(
        VkDevice device,
        const std::vector<VkImageView>& imageViews,
        VkImageView depthImageView,
        VkRenderPass renderPass,
        int32_t width,
        int32_t height
    )
    {
        framebuffers.resize(imageViews.size());

        for (size_t i = 0; i < imageViews.size(); ++i)
        {
            std::array<VkImageView, 2> attachments = { imageViews[i], depthImageView };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = static_cast<uint32_t>(width);
            framebufferInfo.height = static_cast<uint32_t>(height);
            framebufferInfo.layers = 1;

            VkResult result = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]);

            if (result != VK_SUCCESS)
                logger->error("Failed to create framebuffer: {}", static_cast<int>(result));
        }

        logger->info("Created {} framebuffers with depth attachment", framebuffers.size());
    }

    void VulkanFramebufferModule::Destroy(VkDevice device)
    {
        for (auto framebuffer : framebuffers)
            vkDestroyFramebuffer(device, framebuffer, nullptr);

        framebuffers.clear();
        logger->info("Framebuffers destroyed");
    }

    const std::vector<VkFramebuffer>& VulkanFramebufferModule::GetFramebuffers() const
    {
        return framebuffers;
    }

    VkFramebuffer VulkanFramebufferModule::GetFramebuffer(uint32_t index) const
    {
        return framebuffers[index];
    }
}
