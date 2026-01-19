#include "RenderStar/Client/Render/Vulkan/VulkanUtility.hpp"
#include <spdlog/spdlog.h>
#include <vector>
#include <unordered_set>

namespace RenderStar::Client::Render::Vulkan
{
    void VulkanUtility::Check(VkResult result, const std::string& operation)
    {
        if (result != VK_SUCCESS)
            throw std::runtime_error("Vulkan error during " + operation + ": " + TranslateVulkanResult(result));
    }

    std::string VulkanUtility::TranslateVulkanResult(VkResult result)
    {
        switch (result)
        {
            case VK_SUCCESS:                        return "VK_SUCCESS";
            case VK_NOT_READY:                      return "VK_NOT_READY";
            case VK_TIMEOUT:                        return "VK_TIMEOUT";
            case VK_EVENT_SET:                      return "VK_EVENT_SET";
            case VK_EVENT_RESET:                    return "VK_EVENT_RESET";
            case VK_INCOMPLETE:                     return "VK_INCOMPLETE";
            case VK_ERROR_OUT_OF_HOST_MEMORY:       return "VK_ERROR_OUT_OF_HOST_MEMORY";
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:     return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
            case VK_ERROR_INITIALIZATION_FAILED:   return "VK_ERROR_INITIALIZATION_FAILED";
            case VK_ERROR_DEVICE_LOST:              return "VK_ERROR_DEVICE_LOST";
            case VK_ERROR_MEMORY_MAP_FAILED:        return "VK_ERROR_MEMORY_MAP_FAILED";
            case VK_ERROR_LAYER_NOT_PRESENT:        return "VK_ERROR_LAYER_NOT_PRESENT";
            case VK_ERROR_EXTENSION_NOT_PRESENT:    return "VK_ERROR_EXTENSION_NOT_PRESENT";
            case VK_ERROR_FEATURE_NOT_PRESENT:      return "VK_ERROR_FEATURE_NOT_PRESENT";
            case VK_ERROR_INCOMPATIBLE_DRIVER:      return "VK_ERROR_INCOMPATIBLE_DRIVER";
            case VK_ERROR_TOO_MANY_OBJECTS:         return "VK_ERROR_TOO_MANY_OBJECTS";
            case VK_ERROR_FORMAT_NOT_SUPPORTED:     return "VK_ERROR_FORMAT_NOT_SUPPORTED";
            case VK_ERROR_FRAGMENTED_POOL:          return "VK_ERROR_FRAGMENTED_POOL";
            case VK_ERROR_SURFACE_LOST_KHR:         return "VK_ERROR_SURFACE_LOST_KHR";
            case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
            case VK_SUBOPTIMAL_KHR:                 return "VK_SUBOPTIMAL_KHR";
            case VK_ERROR_OUT_OF_DATE_KHR:          return "VK_ERROR_OUT_OF_DATE_KHR";
            default:                                return "Unknown VkResult: " + std::to_string(result);
        }
    }

    bool VulkanUtility::CheckValidationLayerSupport()
    {
        uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        std::unordered_set<std::string> available;
        for (const auto& layer : availableLayers)
            available.insert(layer.layerName);

        return available.find(VALIDATION_LAYER) != available.end();
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL VulkanUtility::DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        auto logger = spdlog::default_logger();

        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            logger->error("Vulkan: {}", pCallbackData->pMessage);
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            logger->warn("Vulkan: {}", pCallbackData->pMessage);
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
            logger->info("Vulkan: {}", pCallbackData->pMessage);
        else
            logger->debug("Vulkan: {}", pCallbackData->pMessage);

        return VK_FALSE;
    }

    VkDebugUtilsMessengerCreateInfoEXT VulkanUtility::CreateDebugMessengerCreateInfo()
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = DebugCallback;
        createInfo.pUserData = nullptr;
        return createInfo;
    }

    VkDebugUtilsMessengerEXT VulkanUtility::CreateDebugMessenger(VkInstance instance)
    {
        auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

        if (func == nullptr)
            throw std::runtime_error("Failed to load vkCreateDebugUtilsMessengerEXT");

        VkDebugUtilsMessengerCreateInfoEXT createInfo = CreateDebugMessengerCreateInfo();
        VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

        VkResult result = func(instance, &createInfo, nullptr, &debugMessenger);
        Check(result, "vkCreateDebugUtilsMessengerEXT");

        return debugMessenger;
    }

    void VulkanUtility::DestroyDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger)
    {
        if (debugMessenger == VK_NULL_HANDLE)
            return;

        auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

        if (func != nullptr)
            func(instance, debugMessenger, nullptr);
    }
}
