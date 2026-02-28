#include "RenderStar/Client/Render/Vulkan/VulkanTextureHandle.hpp"

namespace RenderStar::Client::Render::Vulkan
{
    VulkanTextureHandle::VulkanTextureHandle(
        VkDevice device,
        VmaAllocator allocator,
        VkImage image,
        VmaAllocation allocation,
        VkImageView imageView,
        VkSampler sampler,
        uint32_t width,
        uint32_t height)
        : device(device)
        , allocator(allocator)
        , image(image)
        , allocation(allocation)
        , imageView(imageView)
        , sampler(sampler)
        , width(width)
        , height(height)
    {
    }

    VulkanTextureHandle::~VulkanTextureHandle()
    {
        if (sampler != VK_NULL_HANDLE)
            vkDestroySampler(device, sampler, nullptr);

        if (imageView != VK_NULL_HANDLE)
            vkDestroyImageView(device, imageView, nullptr);

        if (image != VK_NULL_HANDLE)
            vmaDestroyImage(allocator, image, allocation);
    }

    uint32_t VulkanTextureHandle::GetWidth() const
    {
        return width;
    }

    uint32_t VulkanTextureHandle::GetHeight() const
    {
        return height;
    }

    bool VulkanTextureHandle::IsValid() const
    {
        return image != VK_NULL_HANDLE && imageView != VK_NULL_HANDLE && sampler != VK_NULL_HANDLE;
    }

    VkImageView VulkanTextureHandle::GetImageView() const
    {
        return imageView;
    }

    VkSampler VulkanTextureHandle::GetSampler() const
    {
        return sampler;
    }
}
