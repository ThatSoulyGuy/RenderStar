#include "RenderStar/Client/Render/RendererModule.hpp"

#include "RenderStar/Client/Core/ClientWindowModule.hpp"
#include "RenderStar/Client/Event/Buses/ClientRenderEventBus.hpp"
#include "RenderStar/Client/Event/Events/ClientEvents.hpp"
#include "RenderStar/Client/Render/Backend/BackendFactory.hpp"
#include "RenderStar/Client/Render/Backend/IRenderBackend.hpp"
#include "RenderStar/Client/Render/Components/Camera.hpp"
#include "RenderStar/Client/Render/Resource/IBufferHandle.hpp"
#include "RenderStar/Client/Render/Resource/IBufferManager.hpp"
#include "RenderStar/Client/Render/Resource/IMesh.hpp"
#include "RenderStar/Client/Render/Resource/IShaderManager.hpp"
#include "RenderStar/Client/Render/Resource/IShaderProgram.hpp"
#include "RenderStar/Client/Render/Resource/IUniformBindingHandle.hpp"
#include "RenderStar/Client/Render/Resource/IUniformManager.hpp"
#include "RenderStar/Client/Render/Resource/Mesh.hpp"
#include "RenderStar/Client/Render/Resource/StandardUniforms.hpp"
#include "RenderStar/Client/Render/Resource/Vertex.hpp"
#include "RenderStar/Common/Asset/AssetModule.hpp"
#include "RenderStar/Common/Asset/ITextAsset.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <cstring>
#include <vector>

namespace RenderStar::Client::Render
{

    RendererModule::RendererModule() : backend(nullptr), backendType(RenderBackend::OPENGL) { }

    RendererModule::~RendererModule() = default;

    IRenderBackend* RendererModule::GetBackend() const
    {
        return backend.get();
    }

    RenderBackend RendererModule::GetBackendType() const
    {
        return backendType;
    }

    void RendererModule::OnInitialize(Common::Module::ModuleContext& context)
    {
        const auto windowModule = context.GetModule<Core::ClientWindowModule>();

        if (!windowModule.has_value())
        {
            logger->error("ClientWindowModule not found");
            return;
        }

        backendType = windowModule->get().GetPreferredBackend();
        backend = BackendFactory::Create(backendType);

        if (backend == nullptr)
        {
            logger->error("Failed to create render backend");
            return;
        }

        backend->Initialize(windowModule->get().GetWindowHandle(), windowModule->get().GetFramebufferWidth(), windowModule->get().GetFramebufferHeight());

        const auto eventBus = context.GetEventBus<Event::ClientRenderEventBus>();

        if (!eventBus.has_value())
        {
            logger->error("ClientCoreEventBus not found");
            return;
        }

        logger->info("Publishing ClientRendererInitializedEvent with backend={}", static_cast<void*>(backend.get()));
        eventBus.value().get().Publish(Event::Events::ClientRendererInitializedEvent(backend.get()));

        logger->info("RendererModule initialized with {} backend", backendType == RenderBackend::OPENGL ? "OpenGL" : "Vulkan");
    }

    void RendererModule::OnCleanup()
    {
        if (backend != nullptr)
        {
            logger->info("Waiting for GPU to finish before cleanup...");
            backend->WaitIdle();
            backend->Destroy();
        }
    }
}
