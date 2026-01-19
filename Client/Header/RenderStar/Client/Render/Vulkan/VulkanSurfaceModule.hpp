#pragma once

#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>

struct GLFWwindow;

namespace RenderStar::Client::Render::Vulkan
{
    class VulkanSurfaceModule
    {
    public:

        VulkanSurfaceModule();

        ~VulkanSurfaceModule();

        void Create(VkInstance instance, GLFWwindow* window);

        void Destroy(VkInstance instance);

        VkSurfaceKHR GetSurface() const;

    private:

        std::shared_ptr<spdlog::logger> logger;
        VkSurfaceKHR surface;
    };
}
