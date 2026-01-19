#include "RenderStar/Client/Render/Vulkan/VulkanSwapchainModule.hpp"
#include <algorithm>
#include <limits>

namespace RenderStar::Client::Render::Vulkan
{
    VulkanSwapchainModule::VulkanSwapchainModule()
        : logger(spdlog::default_logger())
        , swapchain(VK_NULL_HANDLE)
        , imageFormat(VK_FORMAT_UNDEFINED)
        , width(0)
        , height(0)
    {
    }

    VulkanSwapchainModule::~VulkanSwapchainModule()
    {
    }

    void VulkanSwapchainModule::Create(
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        VkSurfaceKHR surface,
        int32_t windowWidth,
        int32_t windowHeight,
        uint32_t graphicsFamily,
        uint32_t presentFamily
    )
    {
        SwapchainSupportDetails support = QuerySwapchainSupport(physicalDevice, surface);

        VkSurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(support.formats);
        VkPresentModeKHR presentMode = ChoosePresentMode(support.presentModes);
        VkExtent2D extent = ChooseExtent(support.capabilities, windowWidth, windowHeight);

        uint32_t imageCount = support.capabilities.minImageCount + 1;
        if (support.capabilities.maxImageCount > 0 && imageCount > support.capabilities.maxImageCount)
            imageCount = support.capabilities.maxImageCount;

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        uint32_t queueFamilyIndices[] = { graphicsFamily, presentFamily };

        if (graphicsFamily != presentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = support.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        VkResult result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain);

        if (result != VK_SUCCESS)
        {
            logger->error("Failed to create swapchain: {}", static_cast<int>(result));
            return;
        }

        imageFormat = surfaceFormat.format;
        width = static_cast<int32_t>(extent.width);
        height = static_cast<int32_t>(extent.height);

        vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
        images.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapchain, &imageCount, images.data());

        CreateImageViews(device);

        logger->info("Swapchain created: {}x{}, {} images", width, height, images.size());
    }

    void VulkanSwapchainModule::Destroy(VkDevice device)
    {
        for (auto imageView : imageViews)
            vkDestroyImageView(device, imageView, nullptr);
        imageViews.clear();

        if (swapchain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(device, swapchain, nullptr);
            swapchain = VK_NULL_HANDLE;
            logger->info("Swapchain destroyed");
        }

        images.clear();
    }

    VulkanSwapchainModule::SwapchainSupportDetails VulkanSwapchainModule::QuerySwapchainSupport(
        VkPhysicalDevice physicalDevice,
        VkSurfaceKHR surface
    )
    {
        SwapchainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

        if (formatCount > 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

        if (presentModeCount > 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    VkSurfaceFormatKHR VulkanSwapchainModule::ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
    {
        for (const auto& format : formats)
        {
            if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
                format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return format;
        }

        return formats[0];
    }

    VkPresentModeKHR VulkanSwapchainModule::ChoosePresentMode(const std::vector<VkPresentModeKHR>& presentModes)
    {
        (void)presentModes;
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D VulkanSwapchainModule::ChooseExtent(
        const VkSurfaceCapabilitiesKHR& capabilities,
        int32_t windowWidth,
        int32_t windowHeight
    )
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            return capabilities.currentExtent;

        VkExtent2D extent;
        extent.width = std::clamp(
            static_cast<uint32_t>(windowWidth),
            capabilities.minImageExtent.width,
            capabilities.maxImageExtent.width
        );
        extent.height = std::clamp(
            static_cast<uint32_t>(windowHeight),
            capabilities.minImageExtent.height,
            capabilities.maxImageExtent.height
        );

        return extent;
    }

    void VulkanSwapchainModule::CreateImageViews(VkDevice device)
    {
        imageViews.resize(images.size());

        for (size_t i = 0; i < images.size(); ++i)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = images[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = imageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            VkResult result = vkCreateImageView(device, &createInfo, nullptr, &imageViews[i]);

            if (result != VK_SUCCESS)
                logger->error("Failed to create image view: {}", static_cast<int>(result));
        }
    }

    VkSwapchainKHR VulkanSwapchainModule::GetSwapchain() const
    {
        return swapchain;
    }

    const std::vector<VkImage>& VulkanSwapchainModule::GetImages() const
    {
        return images;
    }

    const std::vector<VkImageView>& VulkanSwapchainModule::GetImageViews() const
    {
        return imageViews;
    }

    VkFormat VulkanSwapchainModule::GetImageFormat() const
    {
        return imageFormat;
    }

    int32_t VulkanSwapchainModule::GetWidth() const
    {
        return width;
    }

    int32_t VulkanSwapchainModule::GetHeight() const
    {
        return height;
    }

    uint32_t VulkanSwapchainModule::GetImageCount() const
    {
        return static_cast<uint32_t>(images.size());
    }
}
