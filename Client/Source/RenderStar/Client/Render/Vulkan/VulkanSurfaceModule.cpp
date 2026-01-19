#include "RenderStar/Client/Render/Vulkan/VulkanSurfaceModule.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace RenderStar::Client::Render::Vulkan
{
    VulkanSurfaceModule::VulkanSurfaceModule()
        : logger(spdlog::default_logger())
        , surface(VK_NULL_HANDLE)
    {
    }

    VulkanSurfaceModule::~VulkanSurfaceModule()
    {
    }

    void VulkanSurfaceModule::Create(VkInstance instance, GLFWwindow* window)
    {
        VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &surface);

        if (result != VK_SUCCESS)
        {
            logger->error("Failed to create window surface: {}", static_cast<int>(result));
            return;
        }

        logger->info("Vulkan surface created");
    }

    void VulkanSurfaceModule::Destroy(VkInstance instance)
    {
        if (surface != VK_NULL_HANDLE)
        {
            vkDestroySurfaceKHR(instance, surface, nullptr);
            surface = VK_NULL_HANDLE;
            logger->info("Vulkan surface destroyed");
        }
    }

    VkSurfaceKHR VulkanSurfaceModule::GetSurface() const
    {
        return surface;
    }
}
