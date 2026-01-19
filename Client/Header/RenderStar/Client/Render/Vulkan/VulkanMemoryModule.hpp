#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <spdlog/spdlog.h>

namespace RenderStar::Client::Render::Vulkan
{
    class VulkanMemoryModule
    {
    public:

        VulkanMemoryModule();

        ~VulkanMemoryModule();

        void Create(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);

        void Destroy();

        VmaAllocator GetAllocator() const;

    private:

        std::shared_ptr<spdlog::logger> logger;
        VmaAllocator allocator;
    };
}
