#pragma once

#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>
#include <vector>
#include <string>

namespace RenderStar::Client::Render::Vulkan
{
    class VulkanInstanceModule
    {
    public:

        VulkanInstanceModule();

        ~VulkanInstanceModule();

        void Create(bool enableValidation);

        void Destroy();

        VkInstance GetInstance() const;

        bool IsValidationEnabled() const;

    private:

        bool CheckValidationLayerSupport();

        std::vector<const char*> GetRequiredExtensions(bool enableValidation);

        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData
        );

        void SetupDebugMessenger();

        void DestroyDebugMessenger();

        std::shared_ptr<spdlog::logger> logger;
        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;
        bool validationEnabled;

        static constexpr const char* VALIDATION_LAYER = "VK_LAYER_KHRONOS_validation";
    };
}
