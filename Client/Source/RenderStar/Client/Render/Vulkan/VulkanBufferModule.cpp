#include "RenderStar/Client/Render/Vulkan/VulkanBufferModule.hpp"
#include <cstring>
#include <stdexcept>

namespace RenderStar::Client::Render::Vulkan
{
    VulkanBufferModule::VulkanBufferModule()
        : logger(spdlog::default_logger()->clone("VulkanBufferModule"))
        , device(VK_NULL_HANDLE)
        , allocator(VK_NULL_HANDLE)
        , transferQueue(VK_NULL_HANDLE)
        , transferQueueFamily(0)
        , stagingCommandPool(VK_NULL_HANDLE)
        , stagingCommandBuffer(VK_NULL_HANDLE)
    {
    }

    VulkanBufferModule::~VulkanBufferModule()
    {
        Destroy();
    }

    void VulkanBufferModule::Create(VkDevice deviceHandle, VmaAllocator allocatorHandle, VkQueue queue, uint32_t queueFamily)
    {
        device = deviceHandle;
        allocator = allocatorHandle;
        transferQueue = queue;
        transferQueueFamily = queueFamily;

        CreateStagingResources();

        logger->info("Vulkan buffer module created");
    }

    void VulkanBufferModule::Destroy()
    {
        if (device == VK_NULL_HANDLE)
            return;

        DestroyStagingResources();

        device = VK_NULL_HANDLE;
        allocator = VK_NULL_HANDLE;
        transferQueue = VK_NULL_HANDLE;

        logger->info("Vulkan buffer module destroyed");
    }

    VulkanBuffer VulkanBufferModule::CreateBuffer(VulkanBufferType type, size_t size, bool hostVisible)
    {
        VulkanBuffer result{};
        result.size = size;

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = GetBufferUsageFlags(type);
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = GetMemoryUsage(type, hostVisible);

        if (hostVisible)
            allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        VkResult vkResult = vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &result.buffer, &result.allocation, &result.allocationInfo);

        if (vkResult != VK_SUCCESS)
        {
            logger->error("Failed to create buffer of size {}", size);
            throw std::runtime_error("Failed to create Vulkan buffer");
        }

        return result;
    }

    void VulkanBufferModule::DestroyBuffer(VulkanBuffer& buffer)
    {
        if (buffer.buffer != VK_NULL_HANDLE)
        {
            vmaDestroyBuffer(allocator, buffer.buffer, buffer.allocation);
            buffer.buffer = VK_NULL_HANDLE;
            buffer.allocation = VK_NULL_HANDLE;
        }
    }

    void VulkanBufferModule::UploadBufferData(VulkanBuffer& buffer, const void* data, size_t size, size_t offset)
    {
        void* mapped = MapBuffer(buffer);

        if (mapped)
        {
            std::memcpy(static_cast<char*>(mapped) + offset, data, size);
            UnmapBuffer(buffer);
        }
    }

    void VulkanBufferModule::UploadBufferDataStaged(VulkanBuffer& destinationBuffer, const void* data, size_t size, size_t offset)
    {
        VulkanBuffer stagingBuffer = CreateBuffer(VulkanBufferType::STAGING, size, true);

        std::memcpy(stagingBuffer.allocationInfo.pMappedData, data, size);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(stagingCommandBuffer, &beginInfo);

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = offset;
        copyRegion.size = size;

        vkCmdCopyBuffer(stagingCommandBuffer, stagingBuffer.buffer, destinationBuffer.buffer, 1, &copyRegion);

        vkEndCommandBuffer(stagingCommandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &stagingCommandBuffer;

        vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(transferQueue);

        vkResetCommandBuffer(stagingCommandBuffer, 0);

        DestroyBuffer(stagingBuffer);
    }

    void* VulkanBufferModule::MapBuffer(VulkanBuffer& buffer)
    {
        void* mapped = nullptr;
        VkResult result = vmaMapMemory(allocator, buffer.allocation, &mapped);

        if (result != VK_SUCCESS)
        {
            logger->error("Failed to map buffer memory");
            return nullptr;
        }

        return mapped;
    }

    void VulkanBufferModule::UnmapBuffer(VulkanBuffer& buffer)
    {
        vmaUnmapMemory(allocator, buffer.allocation);
    }

    VkBufferUsageFlags VulkanBufferModule::GetBufferUsageFlags(VulkanBufferType type) const
    {
        switch (type)
        {
            case VulkanBufferType::VERTEX:
                return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            case VulkanBufferType::INDEX:
                return VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            case VulkanBufferType::UNIFORM:
                return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            case VulkanBufferType::STAGING:
                return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            case VulkanBufferType::STORAGE:
                return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        }

        return 0;
    }

    VmaMemoryUsage VulkanBufferModule::GetMemoryUsage(VulkanBufferType type, bool hostVisible) const
    {
        if (hostVisible || type == VulkanBufferType::STAGING || type == VulkanBufferType::UNIFORM)
            return VMA_MEMORY_USAGE_AUTO_PREFER_HOST;

        return VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    }

    void VulkanBufferModule::CreateStagingResources()
    {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = transferQueueFamily;

        if (vkCreateCommandPool(device, &poolInfo, nullptr, &stagingCommandPool) != VK_SUCCESS)
            throw std::runtime_error("Failed to create staging command pool");

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = stagingCommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(device, &allocInfo, &stagingCommandBuffer) != VK_SUCCESS)
            throw std::runtime_error("Failed to allocate staging command buffer");
    }

    void VulkanBufferModule::DestroyStagingResources()
    {
        if (stagingCommandPool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(device, stagingCommandPool, nullptr);
            stagingCommandPool = VK_NULL_HANDLE;
            stagingCommandBuffer = VK_NULL_HANDLE;
        }
    }
}
