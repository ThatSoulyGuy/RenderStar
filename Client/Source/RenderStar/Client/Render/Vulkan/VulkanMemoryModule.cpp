#include "RenderStar/Client/Render/Vulkan/VulkanMemoryModule.hpp"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

namespace RenderStar::Client::Render::Vulkan
{
    VulkanMemoryModule::VulkanMemoryModule()
        : logger(spdlog::default_logger())
        , allocator(VK_NULL_HANDLE)
    {
    }

    VulkanMemoryModule::~VulkanMemoryModule()
    {
        if (allocator != VK_NULL_HANDLE)
            Destroy();
    }

    void VulkanMemoryModule::Create(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device)
    {
        VmaAllocatorCreateInfo allocatorInfo{};
        allocatorInfo.physicalDevice = physicalDevice;
        allocatorInfo.device = device;
        allocatorInfo.instance = instance;
        allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;

        VkResult result = vmaCreateAllocator(&allocatorInfo, &allocator);

        if (result != VK_SUCCESS)
        {
            logger->error("Failed to create VMA allocator: {}", static_cast<int>(result));
            return;
        }

        logger->info("VMA allocator created");
    }

    void VulkanMemoryModule::Destroy()
    {
        if (allocator != VK_NULL_HANDLE)
        {
            vmaDestroyAllocator(allocator);
            allocator = VK_NULL_HANDLE;
            logger->info("VMA allocator destroyed");
        }
    }

    VmaAllocator VulkanMemoryModule::GetAllocator() const
    {
        return allocator;
    }
}
