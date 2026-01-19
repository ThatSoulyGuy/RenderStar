#include "RenderStar/Client/Render/Vulkan/VulkanDeviceModule.hpp"
#include <vector>
#include <set>
#include <cstring>

namespace RenderStar::Client::Render::Vulkan
{
    VulkanDeviceModule::VulkanDeviceModule()
        : logger(spdlog::default_logger())
        , physicalDevice(VK_NULL_HANDLE)
        , device(VK_NULL_HANDLE)
        , graphicsQueue(VK_NULL_HANDLE)
        , presentQueue(VK_NULL_HANDLE)
        , graphicsQueueFamily(0)
        , presentQueueFamily(0)
        , surface(VK_NULL_HANDLE)
    {
    }

    VulkanDeviceModule::~VulkanDeviceModule()
    {
        if (device != VK_NULL_HANDLE)
            Destroy();
    }

    void VulkanDeviceModule::Create(VkInstance instance, VkSurfaceKHR surfaceHandle, bool enableValidation)
    {
        surface = surfaceHandle;
        PickPhysicalDevice(instance, surface);
        CreateLogicalDevice(enableValidation);
    }

    void VulkanDeviceModule::Destroy()
    {
        if (device != VK_NULL_HANDLE)
        {
            vkDestroyDevice(device, nullptr);
            device = VK_NULL_HANDLE;
            logger->info("Logical device destroyed");
        }
    }

    void VulkanDeviceModule::PickPhysicalDevice(VkInstance instance, VkSurfaceKHR surfaceHandle)
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0)
        {
            logger->error("Failed to find GPUs with Vulkan support");
            return;
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto& candidate : devices)
        {
            if (IsDeviceSuitable(candidate, surfaceHandle))
            {
                physicalDevice = candidate;

                VkPhysicalDeviceProperties properties;
                vkGetPhysicalDeviceProperties(physicalDevice, &properties);
                logger->info("Selected GPU: {}", properties.deviceName);

                return;
            }
        }

        logger->error("Failed to find suitable GPU");
    }

    bool VulkanDeviceModule::IsDeviceSuitable(VkPhysicalDevice candidateDevice, VkSurfaceKHR surfaceHandle)
    {
        QueueFamilyIndices indices = FindQueueFamilies(candidateDevice, surfaceHandle);

        if (!indices.IsComplete())
            return false;

        if (!CheckDeviceExtensionSupport(candidateDevice))
            return false;

        graphicsQueueFamily = indices.graphicsFamily.value();
        presentQueueFamily = indices.presentFamily.value();

        return true;
    }

    QueueFamilyIndices VulkanDeviceModule::FindQueueFamilies(VkPhysicalDevice candidateDevice, VkSurfaceKHR surfaceHandle)
    {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(candidateDevice, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(candidateDevice, &queueFamilyCount, queueFamilies.data());

        for (uint32_t i = 0; i < queueFamilyCount; ++i)
        {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                indices.graphicsFamily = i;

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(candidateDevice, i, surfaceHandle, &presentSupport);

            if (presentSupport)
                indices.presentFamily = i;

            if (indices.IsComplete())
                break;
        }

        return indices;
    }

    bool VulkanDeviceModule::CheckDeviceExtensionSupport(VkPhysicalDevice candidateDevice)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(candidateDevice, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(candidateDevice, nullptr, &extensionCount, availableExtensions.data());

        for (const auto& extension : availableExtensions)
        {
            if (strcmp(extension.extensionName, SWAPCHAIN_EXTENSION) == 0)
                return true;
        }

        return false;
    }

    void VulkanDeviceModule::CreateLogicalDevice(bool enableValidation)
    {
        std::set<uint32_t> uniqueQueueFamilies = { graphicsQueueFamily, presentQueueFamily };

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        float queuePriority = 1.0f;

        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = 1;
        createInfo.ppEnabledExtensionNames = &SWAPCHAIN_EXTENSION;

        if (enableValidation)
        {
            createInfo.enabledLayerCount = 1;
            createInfo.ppEnabledLayerNames = &VALIDATION_LAYER;
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }

        VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);

        if (result != VK_SUCCESS)
        {
            logger->error("Failed to create logical device: {}", static_cast<int>(result));
            return;
        }

        vkGetDeviceQueue(device, graphicsQueueFamily, 0, &graphicsQueue);
        vkGetDeviceQueue(device, presentQueueFamily, 0, &presentQueue);

        logger->info("Logical device created");
    }

    VkPhysicalDevice VulkanDeviceModule::GetPhysicalDevice() const
    {
        return physicalDevice;
    }

    VkDevice VulkanDeviceModule::GetDevice() const
    {
        return device;
    }

    VkQueue VulkanDeviceModule::GetGraphicsQueue() const
    {
        return graphicsQueue;
    }

    VkQueue VulkanDeviceModule::GetPresentQueue() const
    {
        return presentQueue;
    }

    uint32_t VulkanDeviceModule::GetGraphicsQueueFamily() const
    {
        return graphicsQueueFamily;
    }

    uint32_t VulkanDeviceModule::GetPresentQueueFamily() const
    {
        return presentQueueFamily;
    }
}
