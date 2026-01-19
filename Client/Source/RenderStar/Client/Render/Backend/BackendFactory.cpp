#include "RenderStar/Client/Render/Backend/BackendFactory.hpp"
#include "RenderStar/Client/Render/OpenGL/OpenGLRenderBackend.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanRenderBackend.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace RenderStar::Client::Render
{
    std::unique_ptr<IRenderBackend> BackendFactory::Create(RenderBackend backendType)
    {
        switch (backendType)
        {
            case RenderBackend::OPENGL:
                return std::make_unique<OpenGL::OpenGLRenderBackend>();
            case RenderBackend::VULKAN:
                return std::make_unique<Vulkan::VulkanRenderBackend>();
            default:
                return nullptr;
        }
    }

    RenderBackend BackendFactory::DetectBestBackend()
    {
        if (IsBackendAvailable(RenderBackend::VULKAN))
            return RenderBackend::VULKAN;

        return RenderBackend::OPENGL;
    }

    bool BackendFactory::IsBackendAvailable(RenderBackend backendType)
    {
        switch (backendType)
        {
            case RenderBackend::VULKAN:
                return glfwVulkanSupported() == GLFW_TRUE;
            case RenderBackend::OPENGL:
                return true;
            default:
                return false;
        }
    }
}
