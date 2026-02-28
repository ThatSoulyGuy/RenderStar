#include "RenderStar/Client/Render/Vulkan/VulkanTextureHandle.hpp"
#include "RenderStar/Client/Render/Resource/IGraphicsResourceManager.hpp"

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
        uint32_t height,
        IGraphicsResourceManager& manager)
        : device(device)
        , allocator(allocator)
        , image(image)
        , allocation(allocation)
        , imageView(imageView)
        , sampler(sampler)
        , width(width)
        , height(height)
    {
        manager.Track(this);
    }

    VulkanTextureHandle::~VulkanTextureHandle()
    {
        if (!released)
            Release();
    }

    void VulkanTextureHandle::Release()
    {
        if (released)
            return;

        if (sampler != VK_NULL_HANDLE)
        {
            vkDestroySampler(device, sampler, nullptr);
            sampler = VK_NULL_HANDLE;
        }

        if (imageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(device, imageView, nullptr);
            imageView = VK_NULL_HANDLE;
        }

        if (image != VK_NULL_HANDLE)
        {
            vmaDestroyImage(allocator, image, allocation);
            image = VK_NULL_HANDLE;
        }

        released = true;
    }

    GraphicsResourceType VulkanTextureHandle::GetResourceType() const
    {
        return GraphicsResourceType::TEXTURE;
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
        return !released && image != VK_NULL_HANDLE && imageView != VK_NULL_HANDLE && sampler != VK_NULL_HANDLE;
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
