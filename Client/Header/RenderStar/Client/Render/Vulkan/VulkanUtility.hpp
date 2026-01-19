#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <stdexcept>

namespace RenderStar::Client::Render::Vulkan
{
    class VulkanUtility
    {
    public:

        static constexpr bool ENABLE_VALIDATION = true;
        static constexpr const char* VALIDATION_LAYER = "VK_LAYER_KHRONOS_validation";

        VulkanUtility() = delete;

        static void Check(VkResult result, const std::string& operation);

        static std::string TranslateVulkanResult(VkResult result);

        static bool CheckValidationLayerSupport();

        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);

        static VkDebugUtilsMessengerCreateInfoEXT CreateDebugMessengerCreateInfo();

        static VkDebugUtilsMessengerEXT CreateDebugMessenger(VkInstance instance);

        static void DestroyDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger);
    };
}
