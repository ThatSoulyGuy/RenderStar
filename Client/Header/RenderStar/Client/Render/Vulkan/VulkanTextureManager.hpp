#pragma once

#include "RenderStar/Client/Render/Resource/ITextureManager.hpp"
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <spdlog/spdlog.h>
#include <memory>

namespace RenderStar::Client::Render::Vulkan
{
    class VulkanTextureManager : public ITextureManager
    {
    public:

        VulkanTextureManager();
        ~VulkanTextureManager() override;

        void Initialize(VkPhysicalDevice physicalDevice, VkDevice device, VmaAllocator allocator, VkQueue graphicsQueue, uint32_t graphicsQueueFamily, IGraphicsResourceManager* resourceManager);
        void Destroy();

        std::unique_ptr<ITextureHandle> CreateFromMemory(
            const TextureDescription& description, const void* pixels) override;

        ITextureHandle* GetDefaultTexture() override;

    private:

        void CreateDefaultTexture();
        void TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
        void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
        void GenerateMipmaps(VkImage image, uint32_t width, uint32_t height, uint32_t mipLevels);
        VkCommandBuffer BeginSingleTimeCommands();
        void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

        VkSamplerAddressMode ToVulkanWrapMode(TextureWrapMode mode) const;
        VkFilter ToVulkanFilter(TextureFilterMode mode) const;

        std::shared_ptr<spdlog::logger> logger;
        VkPhysicalDevice physicalDevice;
        VkDevice device;
        VmaAllocator allocator;
        VkQueue graphicsQueue;
        VkCommandPool commandPool;
        IGraphicsResourceManager* resourceManager;
        std::unique_ptr<ITextureHandle> defaultTexture;
    };
}
