#pragma once

#include "RenderStar/Client/Render/Platform/IRenderTarget.hpp"
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <memory>
#include <spdlog/spdlog.h>

namespace RenderStar::Client::Render::Vulkan
{
    class VulkanRenderTargetAttachment : public ITextureHandle
    {
    public:

        VulkanRenderTargetAttachment(VkImageView imageView, VkSampler sampler, uint32_t width, uint32_t height);

        uint32_t GetWidth() const override;
        uint32_t GetHeight() const override;
        bool IsValid() const override;
        void Release() override;
        GraphicsResourceType GetResourceType() const override;

        VkImageView GetImageView() const;
        VkSampler GetSampler() const;

    private:

        VkImageView imageView;
        VkSampler sampler;
        uint32_t width;
        uint32_t height;
    };

    class VulkanRenderTarget : public Platform::IRenderTarget
    {
    public:

        VulkanRenderTarget(
            VkDevice device,
            VkPhysicalDevice physicalDevice,
            VmaAllocator allocator,
            const Platform::RenderTargetDescription& description);

        ~VulkanRenderTarget() override;

        const std::string& GetName() const override;
        ITextureHandle* GetColorAttachment(uint32_t index) const override;
        ITextureHandle* GetDepthAttachment() const override;
        uint32_t GetWidth() const override;
        uint32_t GetHeight() const override;
        void Resize(uint32_t width, uint32_t height) override;
        Platform::RenderTargetType GetType() const override;
        bool IsSwapchain() const override;
        uint32_t GetSampleCount() const override;

        VkRenderPass GetRenderPass() const;
        VkFramebuffer GetFramebuffer() const;

    private:

        void Create();
        void Destroy();
        void CreateRenderPass();
        VkFormat GetColorFormat() const;

        std::shared_ptr<spdlog::logger> logger;
        VkDevice device;
        VkPhysicalDevice physicalDevice;
        VmaAllocator allocator;
        Platform::RenderTargetDescription description;

        VkImage colorImage = VK_NULL_HANDLE;
        VmaAllocation colorAllocation = VK_NULL_HANDLE;
        VkImageView colorImageView = VK_NULL_HANDLE;
        VkSampler colorSampler = VK_NULL_HANDLE;

        VkImage depthImage = VK_NULL_HANDLE;
        VmaAllocation depthAllocation = VK_NULL_HANDLE;
        VkImageView depthImageView = VK_NULL_HANDLE;

        VkImage msaaColorImage = VK_NULL_HANDLE;
        VmaAllocation msaaColorAllocation = VK_NULL_HANDLE;
        VkImageView msaaColorImageView = VK_NULL_HANDLE;

        VkImage msaaDepthImage = VK_NULL_HANDLE;
        VmaAllocation msaaDepthAllocation = VK_NULL_HANDLE;
        VkImageView msaaDepthImageView = VK_NULL_HANDLE;

        VkRenderPass renderPass = VK_NULL_HANDLE;
        VkFramebuffer framebuffer = VK_NULL_HANDLE;

        std::unique_ptr<VulkanRenderTargetAttachment> colorAttachment;
        std::unique_ptr<VulkanRenderTargetAttachment> depthAttachment;
    };

    class VulkanSwapchainTarget : public Platform::IRenderTarget
    {
    public:

        VulkanSwapchainTarget(uint32_t width, uint32_t height, VkRenderPass renderPass);

        const std::string& GetName() const override;
        ITextureHandle* GetColorAttachment(uint32_t index) const override;
        ITextureHandle* GetDepthAttachment() const override;
        uint32_t GetWidth() const override;
        uint32_t GetHeight() const override;
        void Resize(uint32_t width, uint32_t height) override;
        Platform::RenderTargetType GetType() const override;
        bool IsSwapchain() const override;

        VkRenderPass GetRenderPass() const;

    private:

        std::string name;
        uint32_t width;
        uint32_t height;
        VkRenderPass renderPass;
    };
}
