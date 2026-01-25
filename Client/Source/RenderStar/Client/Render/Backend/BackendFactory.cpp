#include "RenderStar/Client/Render/Backend/BackendFactory.hpp"
#include "RenderStar/Client/Render/OpenGL/OpenGLRenderBackend.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanRenderBackend.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <functional>
#include <unordered_map>

namespace RenderStar::Client::Render
{
    using BackendCreator = std::function<std::unique_ptr<IRenderBackend>()>;
    using BackendAvailabilityChecker = std::function<bool()>;

    struct BackendEntry
    {
        BackendCreator creator;
        BackendAvailabilityChecker checker;
        int32_t priority{};
    };

    static std::unordered_map<RenderBackend, BackendEntry>& GetRegistry()
    {
        static std::unordered_map<RenderBackend, BackendEntry> registry;
        return registry;
    }

    bool BackendFactory::initialized = false;

    void BackendFactory::Initialize()
    {
        if (initialized)
            return;

        auto& registry = GetRegistry();

        registry[RenderBackend::OPENGL] = BackendEntry
        {
            [] { return std::make_unique<OpenGL::OpenGLRenderBackend>(); },
            [] { return true; },
            0
        };

        registry[RenderBackend::VULKAN] = BackendEntry
        {
            [] { return std::make_unique<Vulkan::VulkanRenderBackend>(); },
            [] { return glfwVulkanSupported() == GLFW_TRUE; },
            100
        };

        initialized = true;
    }

    std::unique_ptr<IRenderBackend> BackendFactory::Create(const RenderBackend backendType)
    {
        Initialize();

        auto& registry = GetRegistry();

        if (const auto iterator = registry.find(backendType); iterator != registry.end() && iterator->second.creator)
            return iterator->second.creator();

        return nullptr;
    }

    RenderBackend BackendFactory::DetectBestBackend()
    {
        Initialize();

        auto& registry = GetRegistry();
        auto best = RenderBackend::OPENGL;
        int32_t bestPriority = -1;

        for (const auto& [type, entry] : registry)
        {
            if (entry.checker && entry.checker() && entry.priority > bestPriority)
            {
                best = type;
                bestPriority = entry.priority;
            }
        }

        return best;
    }

    bool BackendFactory::IsBackendAvailable(const RenderBackend backendType)
    {
        Initialize();

        auto& registry = GetRegistry();

        if (const auto iterator = registry.find(backendType); iterator != registry.end() && iterator->second.checker)
            return iterator->second.checker();

        return false;
    }

    std::vector<RenderBackend> BackendFactory::GetAvailableBackends()
    {
        Initialize();

        std::vector<RenderBackend> result;

        for (auto& registry = GetRegistry(); const auto& [type, entry] : registry)
        {
            if (entry.checker && entry.checker())
                result.push_back(type);
        }

        return result;
    }
}
