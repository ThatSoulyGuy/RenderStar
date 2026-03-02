#include "RenderStar/Client/Render/Vulkan/VulkanRenderTarget.hpp"
#include <array>

namespace RenderStar::Client::Render::Vulkan
{
    namespace
    {
        constexpr VkFormat DEPTH_FORMAT = VK_FORMAT_D32_SFLOAT;

        VkSampleCountFlagBits ToVkSampleCount(uint32_t count)
        {
            switch (count)
            {
            case 2: return VK_SAMPLE_COUNT_2_BIT;
            case 4: return VK_SAMPLE_COUNT_4_BIT;
            case 8: return VK_SAMPLE_COUNT_8_BIT;
            case 16: return VK_SAMPLE_COUNT_16_BIT;
            default: return VK_SAMPLE_COUNT_1_BIT;
            }
        }
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
        VkSampleCountFlagBits samples = ToVkSampleCount(description.sampleCount);
        bool msaa = description.sampleCount > 1;

        std::vector<VkAttachmentDescription> attachments;

        VkAttachmentDescription colorAttachmentDesc{};
        colorAttachmentDesc.format = colorFormat;
        colorAttachmentDesc.samples = samples;
        colorAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachmentDesc.storeOp = msaa ? VK_ATTACHMENT_STORE_OP_DONT_CARE : VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentDesc.finalLayout = msaa ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        attachments.push_back(colorAttachmentDesc);

        VkAttachmentReference colorRef{};
        colorRef.attachment = 0;
        colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorRef;

        VkAttachmentDescription depthAttachmentDesc{};
        VkAttachmentReference depthRef{};

        if (description.hasDepth)
        {
            depthAttachmentDesc.format = DEPTH_FORMAT;
            depthAttachmentDesc.samples = samples;
            depthAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            depthAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            depthRef.attachment = static_cast<uint32_t>(attachments.size());
            depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            subpass.pDepthStencilAttachment = &depthRef;
            attachments.push_back(depthAttachmentDesc);
        }

        VkAttachmentDescription resolveAttachmentDesc{};
        VkAttachmentReference resolveRef{};

        if (msaa)
        {
            resolveAttachmentDesc.format = colorFormat;
            resolveAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
            resolveAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            resolveAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            resolveAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            resolveAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            resolveAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            resolveAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            resolveRef.attachment = static_cast<uint32_t>(attachments.size());
            resolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            subpass.pResolveAttachments = &resolveRef;
            attachments.push_back(resolveAttachmentDesc);
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
        VkSampleCountFlagBits samples = ToVkSampleCount(description.sampleCount);
        bool msaa = description.sampleCount > 1;

        if (msaa)
        {
            VkImageCreateInfo msaaColorInfo{};
            msaaColorInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            msaaColorInfo.imageType = VK_IMAGE_TYPE_2D;
            msaaColorInfo.extent = { description.width, description.height, 1 };
            msaaColorInfo.mipLevels = 1;
            msaaColorInfo.arrayLayers = 1;
            msaaColorInfo.format = colorFormat;
            msaaColorInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            msaaColorInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            msaaColorInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
            msaaColorInfo.samples = samples;
            msaaColorInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VmaAllocationCreateInfo msaaColorAllocInfo{};
            msaaColorAllocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
            msaaColorAllocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

            if (vmaCreateImage(allocator, &msaaColorInfo, &msaaColorAllocInfo, &msaaColorImage, &msaaColorAllocation, nullptr) != VK_SUCCESS)
            {
                logger->error("Failed to create MSAA color image for target '{}'", description.name);
                return;
            }

            VkImageViewCreateInfo msaaColorViewInfo{};
            msaaColorViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            msaaColorViewInfo.image = msaaColorImage;
            msaaColorViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            msaaColorViewInfo.format = colorFormat;
            msaaColorViewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

            if (vkCreateImageView(device, &msaaColorViewInfo, nullptr, &msaaColorImageView) != VK_SUCCESS)
            {
                logger->error("Failed to create MSAA color image view for target '{}'", description.name);
                return;
            }
        }

        VkImageCreateInfo colorImageInfo{};
        colorImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        colorImageInfo.imageType = VK_IMAGE_TYPE_2D;
        colorImageInfo.extent = { description.width, description.height, 1 };
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
        colorViewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

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
            depthImageInfo.extent = { description.width, description.height, 1 };
            depthImageInfo.mipLevels = 1;
            depthImageInfo.arrayLayers = 1;
            depthImageInfo.format = DEPTH_FORMAT;
            depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            depthImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            depthImageInfo.samples = msaa ? samples : VK_SAMPLE_COUNT_1_BIT;
            depthImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VmaAllocationCreateInfo depthAllocInfo{};
            depthAllocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

            VkImage* depthTarget = msaa ? &msaaDepthImage : &depthImage;
            VmaAllocation* depthAllocTarget = msaa ? &msaaDepthAllocation : &depthAllocation;

            if (vmaCreateImage(allocator, &depthImageInfo, &depthAllocInfo, depthTarget, depthAllocTarget, nullptr) != VK_SUCCESS)
            {
                logger->error("Failed to create depth image for target '{}'", description.name);
                return;
            }

            VkImageViewCreateInfo depthViewInfo{};
            depthViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            depthViewInfo.image = *depthTarget;
            depthViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            depthViewInfo.format = DEPTH_FORMAT;
            depthViewInfo.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };

            VkImageView* depthViewTarget = msaa ? &msaaDepthImageView : &depthImageView;

            if (vkCreateImageView(device, &depthViewInfo, nullptr, depthViewTarget) != VK_SUCCESS)
            {
                logger->error("Failed to create depth image view for target '{}'", description.name);
                return;
            }

            depthAttachment = std::make_unique<VulkanRenderTargetAttachment>(
                *depthViewTarget, VK_NULL_HANDLE, description.width, description.height);
        }

        std::vector<VkImageView> fbAttachments;

        if (msaa)
        {
            fbAttachments.push_back(msaaColorImageView);

            if (description.hasDepth)
                fbAttachments.push_back(msaaDepthImageView);

            fbAttachments.push_back(colorImageView);
        }
        else
        {
            fbAttachments.push_back(colorImageView);

            if (description.hasDepth)
                fbAttachments.push_back(depthImageView);
        }

        VkFramebufferCreateInfo fbInfo{};
        fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbInfo.renderPass = renderPass;
        fbInfo.attachmentCount = static_cast<uint32_t>(fbAttachments.size());
        fbInfo.pAttachments = fbAttachments.data();
        fbInfo.width = description.width;
        fbInfo.height = description.height;
        fbInfo.layers = 1;

        if (vkCreateFramebuffer(device, &fbInfo, nullptr, &framebuffer) != VK_SUCCESS)
        {
            logger->error("Failed to create framebuffer for target '{}'", description.name);
            return;
        }

        logger->info("Created Vulkan render target '{}' {}x{} ({}x MSAA)", description.name, description.width, description.height, description.sampleCount);
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

        if (msaaColorImageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(device, msaaColorImageView, nullptr);
            msaaColorImageView = VK_NULL_HANDLE;
        }

        if (msaaColorImage != VK_NULL_HANDLE)
        {
            vmaDestroyImage(allocator, msaaColorImage, msaaColorAllocation);
            msaaColorImage = VK_NULL_HANDLE;
            msaaColorAllocation = VK_NULL_HANDLE;
        }

        if (msaaDepthImageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(device, msaaDepthImageView, nullptr);
            msaaDepthImageView = VK_NULL_HANDLE;
        }

        if (msaaDepthImage != VK_NULL_HANDLE)
        {
            vmaDestroyImage(allocator, msaaDepthImage, msaaDepthAllocation);
            msaaDepthImage = VK_NULL_HANDLE;
            msaaDepthAllocation = VK_NULL_HANDLE;
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

    uint32_t VulkanRenderTarget::GetSampleCount() const
    {
        return description.sampleCount;
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
