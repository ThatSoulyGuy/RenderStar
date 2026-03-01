#include "RenderStar/Client/Render/Vulkan/VulkanRenderTarget.hpp"
#include <array>

namespace RenderStar::Client::Render::Vulkan
{
    namespace
    {
        constexpr VkFormat DEPTH_FORMAT = VK_FORMAT_D32_SFLOAT;
    }

    VulkanRenderTargetAttachment::VulkanRenderTargetAttachment(
        VkImageView imageView, VkSampler sampler, uint32_t width, uint32_t height)
        : imageView(imageView)
        , sampler(sampler)
        , width(width)
        , height(height)
    {
    }

    uint32_t VulkanRenderTargetAttachment::GetWidth() const { return width; }
    uint32_t VulkanRenderTargetAttachment::GetHeight() const { return height; }
    bool VulkanRenderTargetAttachment::IsValid() const { return imageView != VK_NULL_HANDLE; }
    void VulkanRenderTargetAttachment::Release() {}
    GraphicsResourceType VulkanRenderTargetAttachment::GetResourceType() const { return GraphicsResourceType::TEXTURE; }
    VkImageView VulkanRenderTargetAttachment::GetImageView() const { return imageView; }
    VkSampler VulkanRenderTargetAttachment::GetSampler() const { return sampler; }

    VulkanRenderTarget::VulkanRenderTarget(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        VmaAllocator allocator,
        const Platform::RenderTargetDescription& description)
        : logger(spdlog::default_logger()->clone("VulkanRenderTarget"))
        , device(device)
        , physicalDevice(physicalDevice)
        , allocator(allocator)
        , description(description)
    {
        CreateRenderPass();
        Create();
    }

    VulkanRenderTarget::~VulkanRenderTarget()
    {
        Destroy();

        if (renderPass != VK_NULL_HANDLE)
        {
            vkDestroyRenderPass(device, renderPass, nullptr);
            renderPass = VK_NULL_HANDLE;
        }
    }

    VkFormat VulkanRenderTarget::GetColorFormat() const
    {
        switch (description.colorFormat)
        {
        case TextureFormat::RGBA8: return VK_FORMAT_B8G8R8A8_UNORM;
        case TextureFormat::RGBA16F: return VK_FORMAT_R16G16B16A16_SFLOAT;
        case TextureFormat::RGBA32F: return VK_FORMAT_R32G32B32A32_SFLOAT;
        default: return VK_FORMAT_B8G8R8A8_UNORM;
        }
    }

    void VulkanRenderTarget::CreateRenderPass()
    {
        VkFormat colorFormat = GetColorFormat();

        VkAttachmentDescription colorAttachmentDesc{};
        colorAttachmentDesc.format = colorFormat;
        colorAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentReference colorRef{};
        colorRef.attachment = 0;
        colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        std::vector<VkAttachmentDescription> attachments = { colorAttachmentDesc };

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorRef;

        VkAttachmentDescription depthAttachmentDesc{};
        VkAttachmentReference depthRef{};

        if (description.hasDepth)
        {
            depthAttachmentDesc.format = DEPTH_FORMAT;
            depthAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
            depthAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            depthAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            depthRef.attachment = 1;
            depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            subpass.pDepthStencilAttachment = &depthRef;
            attachments.push_back(depthAttachmentDesc);
        }

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        VkResult result = vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);

        if (result != VK_SUCCESS)
            logger->error("Failed to create render pass for target '{}': {}", description.name, static_cast<int>(result));
    }

    void VulkanRenderTarget::Create()
    {
        VkFormat colorFormat = GetColorFormat();

        VkImageCreateInfo colorImageInfo{};
        colorImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        colorImageInfo.imageType = VK_IMAGE_TYPE_2D;
        colorImageInfo.extent.width = description.width;
        colorImageInfo.extent.height = description.height;
        colorImageInfo.extent.depth = 1;
        colorImageInfo.mipLevels = 1;
        colorImageInfo.arrayLayers = 1;
        colorImageInfo.format = colorFormat;
        colorImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        colorImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        colorImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        colorImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo colorAllocInfo{};
        colorAllocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

        if (vmaCreateImage(allocator, &colorImageInfo, &colorAllocInfo, &colorImage, &colorAllocation, nullptr) != VK_SUCCESS)
        {
            logger->error("Failed to create color image for target '{}'", description.name);
            return;
        }

        VkImageViewCreateInfo colorViewInfo{};
        colorViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        colorViewInfo.image = colorImage;
        colorViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        colorViewInfo.format = colorFormat;
        colorViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        colorViewInfo.subresourceRange.baseMipLevel = 0;
        colorViewInfo.subresourceRange.levelCount = 1;
        colorViewInfo.subresourceRange.baseArrayLayer = 0;
        colorViewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &colorViewInfo, nullptr, &colorImageView) != VK_SUCCESS)
        {
            logger->error("Failed to create color image view for target '{}'", description.name);
            return;
        }

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        if (vkCreateSampler(device, &samplerInfo, nullptr, &colorSampler) != VK_SUCCESS)
        {
            logger->error("Failed to create sampler for target '{}'", description.name);
            return;
        }

        colorAttachment = std::make_unique<VulkanRenderTargetAttachment>(
            colorImageView, colorSampler, description.width, description.height);

        if (description.hasDepth)
        {
            VkImageCreateInfo depthImageInfo{};
            depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
            depthImageInfo.extent.width = description.width;
            depthImageInfo.extent.height = description.height;
            depthImageInfo.extent.depth = 1;
            depthImageInfo.mipLevels = 1;
            depthImageInfo.arrayLayers = 1;
            depthImageInfo.format = DEPTH_FORMAT;
            depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            depthImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            depthImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VmaAllocationCreateInfo depthAllocInfo{};
            depthAllocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

            if (vmaCreateImage(allocator, &depthImageInfo, &depthAllocInfo, &depthImage, &depthAllocation, nullptr) != VK_SUCCESS)
            {
                logger->error("Failed to create depth image for target '{}'", description.name);
                return;
            }

            VkImageViewCreateInfo depthViewInfo{};
            depthViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            depthViewInfo.image = depthImage;
            depthViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            depthViewInfo.format = DEPTH_FORMAT;
            depthViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            depthViewInfo.subresourceRange.baseMipLevel = 0;
            depthViewInfo.subresourceRange.levelCount = 1;
            depthViewInfo.subresourceRange.baseArrayLayer = 0;
            depthViewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device, &depthViewInfo, nullptr, &depthImageView) != VK_SUCCESS)
            {
                logger->error("Failed to create depth image view for target '{}'", description.name);
                return;
            }

            depthAttachment = std::make_unique<VulkanRenderTargetAttachment>(
                depthImageView, VK_NULL_HANDLE, description.width, description.height);
        }

        std::vector<VkImageView> attachments = { colorImageView };

        if (description.hasDepth)
            attachments.push_back(depthImageView);

        VkFramebufferCreateInfo fbInfo{};
        fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbInfo.renderPass = renderPass;
        fbInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        fbInfo.pAttachments = attachments.data();
        fbInfo.width = description.width;
        fbInfo.height = description.height;
        fbInfo.layers = 1;

        if (vkCreateFramebuffer(device, &fbInfo, nullptr, &framebuffer) != VK_SUCCESS)
        {
            logger->error("Failed to create framebuffer for target '{}'", description.name);
            return;
        }

        logger->info("Created Vulkan render target '{}' {}x{}", description.name, description.width, description.height);
    }

    void VulkanRenderTarget::Destroy()
    {
        colorAttachment.reset();
        depthAttachment.reset();

        if (framebuffer != VK_NULL_HANDLE)
        {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
            framebuffer = VK_NULL_HANDLE;
        }

        if (colorSampler != VK_NULL_HANDLE)
        {
            vkDestroySampler(device, colorSampler, nullptr);
            colorSampler = VK_NULL_HANDLE;
        }

        if (colorImageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(device, colorImageView, nullptr);
            colorImageView = VK_NULL_HANDLE;
        }

        if (colorImage != VK_NULL_HANDLE)
        {
            vmaDestroyImage(allocator, colorImage, colorAllocation);
            colorImage = VK_NULL_HANDLE;
            colorAllocation = VK_NULL_HANDLE;
        }

        if (depthImageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(device, depthImageView, nullptr);
            depthImageView = VK_NULL_HANDLE;
        }

        if (depthImage != VK_NULL_HANDLE)
        {
            vmaDestroyImage(allocator, depthImage, depthAllocation);
            depthImage = VK_NULL_HANDLE;
            depthAllocation = VK_NULL_HANDLE;
        }
    }

    const std::string& VulkanRenderTarget::GetName() const
    {
        return description.name;
    }

    ITextureHandle* VulkanRenderTarget::GetColorAttachment(uint32_t index) const
    {
        if (index == 0 && colorAttachment)
            return colorAttachment.get();

        return nullptr;
    }

    ITextureHandle* VulkanRenderTarget::GetDepthAttachment() const
    {
        return depthAttachment.get();
    }

    uint32_t VulkanRenderTarget::GetWidth() const
    {
        return description.width;
    }

    uint32_t VulkanRenderTarget::GetHeight() const
    {
        return description.height;
    }

    void VulkanRenderTarget::Resize(uint32_t width, uint32_t height)
    {
        if (width == description.width && height == description.height)
            return;

        description.width = width;
        description.height = height;

        Destroy();
        Create();

        logger->debug("Resized Vulkan render target '{}' to {}x{}", description.name, width, height);
    }

    Platform::RenderTargetType VulkanRenderTarget::GetType() const
    {
        if (description.hasDepth)
            return Platform::RenderTargetType::COLOR_DEPTH;

        return Platform::RenderTargetType::COLOR_ONLY;
    }

    bool VulkanRenderTarget::IsSwapchain() const
    {
        return false;
    }

    VkRenderPass VulkanRenderTarget::GetRenderPass() const
    {
        return renderPass;
    }

    VkFramebuffer VulkanRenderTarget::GetFramebuffer() const
    {
        return framebuffer;
    }

    VulkanSwapchainTarget::VulkanSwapchainTarget(uint32_t width, uint32_t height, VkRenderPass renderPass)
        : name("SWAPCHAIN")
        , width(width)
        , height(height)
        , renderPass(renderPass)
    {
    }

    const std::string& VulkanSwapchainTarget::GetName() const
    {
        return name;
    }

    ITextureHandle* VulkanSwapchainTarget::GetColorAttachment(uint32_t) const
    {
        return nullptr;
    }

    ITextureHandle* VulkanSwapchainTarget::GetDepthAttachment() const
    {
        return nullptr;
    }

    uint32_t VulkanSwapchainTarget::GetWidth() const
    {
        return width;
    }

    uint32_t VulkanSwapchainTarget::GetHeight() const
    {
        return height;
    }

    void VulkanSwapchainTarget::Resize(uint32_t newWidth, uint32_t newHeight)
    {
        width = newWidth;
        height = newHeight;
    }

    Platform::RenderTargetType VulkanSwapchainTarget::GetType() const
    {
        return Platform::RenderTargetType::SWAPCHAIN;
    }

    bool VulkanSwapchainTarget::IsSwapchain() const
    {
        return true;
    }

    VkRenderPass VulkanSwapchainTarget::GetRenderPass() const
    {
        return renderPass;
    }
}
