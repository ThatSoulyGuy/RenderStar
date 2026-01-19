#pragma once

#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>
#include <optional>

namespace RenderStar::Client::Render::Vulkan
{
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool IsComplete() const
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    class VulkanDeviceModule
    {
    public:

        VulkanDeviceModule();

        ~VulkanDeviceModule();

        void Create(VkInstance instance, VkSurfaceKHR surface, bool enableValidation);

        void Destroy();

        VkPhysicalDevice GetPhysicalDevice() const;

        VkDevice GetDevice() const;

        VkQueue GetGraphicsQueue() const;

        VkQueue GetPresentQueue() const;

        uint32_t GetGraphicsQueueFamily() const;

        uint32_t GetPresentQueueFamily() const;

    private:

        void PickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);

        bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);

        QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

        bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

        void CreateLogicalDevice(bool enableValidation);

        std::shared_ptr<spdlog::logger> logger;
        VkPhysicalDevice physicalDevice;
        VkDevice device;
        VkQueue graphicsQueue;
        VkQueue presentQueue;
        uint32_t graphicsQueueFamily;
        uint32_t presentQueueFamily;
        VkSurfaceKHR surface;

        static constexpr const char* SWAPCHAIN_EXTENSION = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
        static constexpr const char* VALIDATION_LAYER = "VK_LAYER_KHRONOS_validation";
    };
}
