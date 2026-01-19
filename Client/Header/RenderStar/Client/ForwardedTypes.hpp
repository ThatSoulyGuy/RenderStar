#pragma once

#include <cstdint>
#include <memory>

namespace RenderStar::Client::Core
{
    class ClientWindowModule;
    class ClientLifecycleModule;
}

namespace RenderStar::Client::Render
{
    class IRenderBackend;
    class RendererModule;
    enum class RenderBackend : int32_t;
}

namespace RenderStar::Client::Render::OpenGL
{
    class OpenGLRenderBackend;
    class OpenGLBufferManager;
    class OpenGLShaderProgram;
    class OpenGLMesh;
    class OpenGLUniformManager;
}

namespace RenderStar::Client::Render::Vulkan
{
    class VulkanRenderBackend;
}

namespace RenderStar::Client::Render::Components
{
    struct Camera;
    struct MeshRenderer;
}

namespace RenderStar::Client::Event
{
    class ClientCoreEventBus;
    class ClientRenderEventBus;
}
