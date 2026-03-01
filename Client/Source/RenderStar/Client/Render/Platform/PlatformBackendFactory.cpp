#include "RenderStar/Client/Render/Platform/PlatformBackendFactory.hpp"
#include "RenderStar/Client/Render/Backend/IRenderBackend.hpp"
#include "RenderStar/Client/Render/Backend/RenderBackend.hpp"
#include "RenderStar/Client/Render/OpenGL/OpenGLPlatformBackend.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanPlatformBackend.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanRenderBackend.hpp"
#include <spdlog/spdlog.h>

namespace RenderStar::Client::Render::Platform
{
    std::unique_ptr<IRenderingPlatformBackend> PlatformBackendFactory::Create(IRenderBackend* backend)
    {
        switch (backend->GetType())
        {
        case RenderBackend::OPENGL:
            return std::make_unique<OpenGL::OpenGLPlatformBackend>(backend);
        case RenderBackend::VULKAN:
            return std::make_unique<Vulkan::VulkanPlatformBackend>(
                static_cast<Vulkan::VulkanRenderBackend*>(backend));
        }

        return nullptr;
    }
}
