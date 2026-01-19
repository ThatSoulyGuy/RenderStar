#include "RenderStar/Client/Render/Vulkan/VulkanInstanceModule.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace RenderStar::Client::Render::Vulkan
{
    VulkanInstanceModule::VulkanInstanceModule()
        : logger(spdlog::default_logger())
        , instance(VK_NULL_HANDLE)
        , debugMessenger(VK_NULL_HANDLE)
        , validationEnabled(false)
    {
    }

    VulkanInstanceModule::~VulkanInstanceModule()
    {
        if (instance != VK_NULL_HANDLE)
            Destroy();
    }

    void VulkanInstanceModule::Create(bool enableValidation)
    {
        validationEnabled = enableValidation && CheckValidationLayerSupport();

        if (enableValidation && !validationEnabled)
            logger->warn("Validation layers requested but not available");

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "RenderStar";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "RenderStar Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        auto extensions = GetRequiredExtensions(validationEnabled);

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

        if (validationEnabled)
        {
            createInfo.enabledLayerCount = 1;
            createInfo.ppEnabledLayerNames = &VALIDATION_LAYER;

            debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debugCreateInfo.messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debugCreateInfo.messageType =
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugCreateInfo.pfnUserCallback = DebugCallback;
            debugCreateInfo.pUserData = this;

            createInfo.pNext = &debugCreateInfo;

            logger->info("Validation layers enabled");
        }
        else
        {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
            logger->info("Validation layers disabled");
        }

        VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

        if (result != VK_SUCCESS)
        {
            logger->error("Failed to create Vulkan instance: {}", static_cast<int>(result));
            return;
        }

        if (validationEnabled)
            SetupDebugMessenger();

        logger->info("Vulkan instance created");
    }

    void VulkanInstanceModule::Destroy()
    {
        if (validationEnabled)
            DestroyDebugMessenger();

        if (instance != VK_NULL_HANDLE)
        {
            vkDestroyInstance(instance, nullptr);
            instance = VK_NULL_HANDLE;
            logger->info("Vulkan instance destroyed");
        }
    }

    VkInstance VulkanInstanceModule::GetInstance() const
    {
        return instance;
    }

    bool VulkanInstanceModule::IsValidationEnabled() const
    {
        return validationEnabled;
    }

    bool VulkanInstanceModule::CheckValidationLayerSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(VALIDATION_LAYER, layerProperties.layerName) == 0)
                return true;
        }

        return false;
    }

    std::vector<const char*> VulkanInstanceModule::GetRequiredExtensions(bool enableValidation)
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidation)
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        return extensions;
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL VulkanInstanceModule::DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    )
    {
        auto* module = static_cast<VulkanInstanceModule*>(pUserData);

        if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            module->logger->error("Vulkan: {}", pCallbackData->pMessage);
        else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            module->logger->warn("Vulkan: {}", pCallbackData->pMessage);
        else
            module->logger->debug("Vulkan: {}", pCallbackData->pMessage);

        return VK_FALSE;
    }

    void VulkanInstanceModule::SetupDebugMessenger()
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
        createInfo.pUserData = this;

        auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

        if (func != nullptr)
        {
            VkResult result = func(instance, &createInfo, nullptr, &debugMessenger);
            if (result != VK_SUCCESS)
                logger->warn("Failed to set up debug messenger");
        }
        else
        {
            logger->warn("vkCreateDebugUtilsMessengerEXT not available");
        }
    }

    void VulkanInstanceModule::DestroyDebugMessenger()
    {
        if (debugMessenger == VK_NULL_HANDLE)
            return;

        auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

        if (func != nullptr)
            func(instance, debugMessenger, nullptr);

        debugMessenger = VK_NULL_HANDLE;
    }
}
