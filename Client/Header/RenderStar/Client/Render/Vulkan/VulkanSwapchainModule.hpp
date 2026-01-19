#pragma once

#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>
#include <vector>

namespace RenderStar::Client::Render::Vulkan
{
    class VulkanSwapchainModule
    {
    public:

        VulkanSwapchainModule();

        ~VulkanSwapchainModule();

        void Create(
            VkPhysicalDevice physicalDevice,
            VkDevice device,
            VkSurfaceKHR surface,
            int32_t windowWidth,
            int32_t windowHeight,
            uint32_t graphicsFamily,
            uint32_t presentFamily
        );

        void Destroy(VkDevice device);

        VkSwapchainKHR GetSwapchain() const;

        const std::vector<VkImage>& GetImages() const;

        const std::vector<VkImageView>& GetImageViews() const;

        VkFormat GetImageFormat() const;

        int32_t GetWidth() const;

        int32_t GetHeight() const;

        uint32_t GetImageCount() const;

    private:

        struct SwapchainSupportDetails
        {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        };

        SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

        VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);

        VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& presentModes);

        VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, int32_t width, int32_t height);

        void CreateImageViews(VkDevice device);

        std::shared_ptr<spdlog::logger> logger;
        VkSwapchainKHR swapchain;
        std::vector<VkImage> images;
        std::vector<VkImageView> imageViews;
        VkFormat imageFormat;
        int32_t width;
        int32_t height;
    };
}
