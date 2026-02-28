#include "RenderStar/Client/Render/Vulkan/VulkanTextureManager.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanTextureHandle.hpp"
#include "RenderStar/Client/Render/Resource/IGraphicsResourceManager.hpp"
#include <cstring>
#include <stdexcept>

namespace RenderStar::Client::Render::Vulkan
{
    VulkanTextureManager::VulkanTextureManager()
        : logger(spdlog::default_logger()->clone("VulkanTextureManager"))
        , device(VK_NULL_HANDLE)
        , allocator(VK_NULL_HANDLE)
        , graphicsQueue(VK_NULL_HANDLE)
        , commandPool(VK_NULL_HANDLE)
        , resourceManager(nullptr)
    {
    }

    VulkanTextureManager::~VulkanTextureManager()
    {
        Destroy();
    }

    void VulkanTextureManager::Initialize(VkDevice vulkanDevice, VmaAllocator vmaAllocator, VkQueue queue, uint32_t queueFamily, IGraphicsResourceManager* manager)
    {
        device = vulkanDevice;
        allocator = vmaAllocator;
        graphicsQueue = queue;
        resourceManager = manager;

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        poolInfo.queueFamilyIndex = queueFamily;

        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
            throw std::runtime_error("Failed to create texture command pool");

        CreateDefaultTexture();

        logger->info("VulkanTextureManager initialized");
    }

    void VulkanTextureManager::Destroy()
    {
        defaultTexture.reset();

        if (commandPool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(device, commandPool, nullptr);
            commandPool = VK_NULL_HANDLE;
        }

        device = VK_NULL_HANDLE;
    }

    std::unique_ptr<ITextureHandle> VulkanTextureManager::CreateFromMemory(
        const TextureDescription& description, const void* pixels)
    {
        uint32_t w = description.width;
        uint32_t h = description.height;
        VkDeviceSize imageSize = static_cast<VkDeviceSize>(w) * h * 4;

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = imageSize;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo stagingAllocInfo{};
        stagingAllocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
        stagingAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

        VkBuffer stagingBuffer;
        VmaAllocation stagingAllocation;

        if (vmaCreateBuffer(allocator, &bufferInfo, &stagingAllocInfo, &stagingBuffer, &stagingAllocation, nullptr) != VK_SUCCESS)
        {
            logger->error("Failed to create staging buffer for texture");
            return nullptr;
        }

        void* mapped;
        vmaMapMemory(allocator, stagingAllocation, &mapped);
        std::memcpy(mapped, pixels, imageSize);
        vmaUnmapMemory(allocator, stagingAllocation);

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = w;
        imageInfo.extent.height = h;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo imageAllocInfo{};
        imageAllocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

        VkImage image;
        VmaAllocation imageAllocation;

        if (vmaCreateImage(allocator, &imageInfo, &imageAllocInfo, &image, &imageAllocation, nullptr) != VK_SUCCESS)
        {
            logger->error("Failed to create texture image");
            vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
            return nullptr;
        }

        TransitionImageLayout(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        CopyBufferToImage(stagingBuffer, image, w, h);
        TransitionImageLayout(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;

        if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
        {
            logger->error("Failed to create texture image view");
            vmaDestroyImage(allocator, image, imageAllocation);
            return nullptr;
        }

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = ToVulkanFilter(description.magFilter);
        samplerInfo.minFilter = ToVulkanFilter(description.minFilter);
        samplerInfo.addressModeU = ToVulkanWrapMode(description.wrapS);
        samplerInfo.addressModeV = ToVulkanWrapMode(description.wrapT);
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        VkSampler sampler;

        if (vkCreateSampler(device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
        {
            logger->error("Failed to create texture sampler");
            vkDestroyImageView(device, imageView, nullptr);
            vmaDestroyImage(allocator, image, imageAllocation);
            return nullptr;
        }

        logger->info("Created texture {}x{}", w, h);

        return std::make_unique<VulkanTextureHandle>(device, allocator, image, imageAllocation, imageView, sampler, w, h, *resourceManager);
    }

    ITextureHandle* VulkanTextureManager::GetDefaultTexture()
    {
        return defaultTexture.get();
    }

    void VulkanTextureManager::CreateDefaultTexture()
    {
        uint32_t white = 0xFFFFFFFF;

        TextureDescription desc;
        desc.width = 1;
        desc.height = 1;
        desc.minFilter = TextureFilterMode::NEAREST;
        desc.magFilter = TextureFilterMode::NEAREST;

        defaultTexture = CreateFromMemory(desc, &white);
    }

    void VulkanTextureManager::TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkCommandBuffer cmd = BeginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags srcStage;
        VkPipelineStageFlags dstStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else
        {
            srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
            dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        }

        vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        EndSingleTimeCommands(cmd);
    }

    void VulkanTextureManager::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
    {
        VkCommandBuffer cmd = BeginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {width, height, 1};

        vkCmdCopyBufferToImage(cmd, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        EndSingleTimeCommands(cmd);
    }

    VkCommandBuffer VulkanTextureManager::BeginSingleTimeCommands()
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void VulkanTextureManager::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);

        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }

    VkSamplerAddressMode VulkanTextureManager::ToVulkanWrapMode(TextureWrapMode mode) const
    {
        switch (mode)
        {
            case TextureWrapMode::REPEAT: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            case TextureWrapMode::CLAMP_TO_EDGE: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        }

        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }

    VkFilter VulkanTextureManager::ToVulkanFilter(TextureFilterMode mode) const
    {
        switch (mode)
        {
            case TextureFilterMode::NEAREST: return VK_FILTER_NEAREST;
            case TextureFilterMode::LINEAR: return VK_FILTER_LINEAR;
        }

        return VK_FILTER_LINEAR;
    }
}
