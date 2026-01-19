#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <spdlog/spdlog.h>
#include <cstddef>
#include <vector>

namespace RenderStar::Client::Render::Vulkan
{
    struct VulkanBuffer
    {
        VkBuffer buffer;
        VmaAllocation allocation;
        VmaAllocationInfo allocationInfo;
        size_t size;
    };

    enum class VulkanBufferType
    {
        VERTEX,
        INDEX,
        UNIFORM,
        STAGING,
        STORAGE
    };

    class VulkanBufferModule
    {
    public:

        VulkanBufferModule();
        ~VulkanBufferModule();

        void Create(VkDevice device, VmaAllocator allocator, VkQueue transferQueue, uint32_t transferQueueFamily);
        void Destroy();

        VulkanBuffer CreateBuffer(
            VulkanBufferType type,
            size_t size,
            bool hostVisible = false);

        void DestroyBuffer(VulkanBuffer& buffer);

        void UploadBufferData(
            VulkanBuffer& buffer,
            const void* data,
            size_t size,
            size_t offset = 0);

        void UploadBufferDataStaged(
            VulkanBuffer& destinationBuffer,
            const void* data,
            size_t size,
            size_t offset = 0);

        void* MapBuffer(VulkanBuffer& buffer);
        void UnmapBuffer(VulkanBuffer& buffer);

    private:

        VkBufferUsageFlags GetBufferUsageFlags(VulkanBufferType type) const;
        VmaMemoryUsage GetMemoryUsage(VulkanBufferType type, bool hostVisible) const;

        void CreateStagingResources();
        void DestroyStagingResources();

        std::shared_ptr<spdlog::logger> logger;
        VkDevice device;
        VmaAllocator allocator;
        VkQueue transferQueue;
        uint32_t transferQueueFamily;
        VkCommandPool stagingCommandPool;
        VkCommandBuffer stagingCommandBuffer;
    };
}
