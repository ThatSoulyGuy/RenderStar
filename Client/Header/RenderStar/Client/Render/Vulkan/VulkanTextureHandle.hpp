#pragma once

#include "RenderStar/Client/Render/Resource/ITextureHandle.hpp"
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace RenderStar::Client::Render::Vulkan
{
    class VulkanTextureHandle : public ITextureHandle
    {
    public:

        VulkanTextureHandle(
            VkDevice device,
            VmaAllocator allocator,
            VkImage image,
            VmaAllocation allocation,
            VkImageView imageView,
            VkSampler sampler,
            uint32_t width,
            uint32_t height,
            IGraphicsResourceManager& resourceManager);

        ~VulkanTextureHandle() override;

        void Release() override;
        GraphicsResourceType GetResourceType() const override;

        uint32_t GetWidth() const override;
        uint32_t GetHeight() const override;
        bool IsValid() const override;

        VkImageView GetImageView() const;
        VkSampler GetSampler() const;

    private:

        VkDevice device;
        VmaAllocator allocator;
        VkImage image;
        VmaAllocation allocation;
        VkImageView imageView;
        VkSampler sampler;
        uint32_t width;
        uint32_t height;
    };
}
