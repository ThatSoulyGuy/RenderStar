#include "RenderStar/Client/Core/ClientLifecycleModule.hpp"
#include "RenderStar/Client/Core/ClientWindowModule.hpp"
#include "RenderStar/Client/Gameplay/ClientPlayerModule.hpp"
#include "RenderStar/Client/Gameplay/PlayerController.hpp"
#include "RenderStar/Client/Gameplay/PlayerControllerAffector.hpp"
#include "RenderStar/Client/Input/ClientInputModule.hpp"
#include "RenderStar/Client/Event/Buses/ClientCoreEventBus.hpp"
#include "RenderStar/Client/Event/Buses/ClientRenderEventBus.hpp"
#include "RenderStar/Client/Event/Events/ClientEvents.hpp"
#include "RenderStar/Client/Render/Affectors/CameraAffector.hpp"
#include "RenderStar/Client/Render/Backend/IRenderBackend.hpp"
#include "RenderStar/Client/Render/Components/Camera.hpp"
#include "RenderStar/Client/Render/RendererModule.hpp"
#include "RenderStar/Client/Render/Resource/IBufferManager.hpp"
#include "RenderStar/Client/Render/Resource/IShaderManager.hpp"
#include "RenderStar/Client/Render/Resource/IUniformManager.hpp"
#include "RenderStar/Client/Render/Resource/Mesh.hpp"
#include "RenderStar/Client/Render/Resource/StandardUniforms.hpp"
#include "RenderStar/Client/Render/Resource/Vertex.hpp"
#include "RenderStar/Common/Asset/AssetModule.hpp"
#include "RenderStar/Common/Component/Affectors/TransformAffector.hpp"
#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Component/Components/Transform.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"
#include "RenderStar/Common/Time/TimeModule.hpp"

using namespace RenderStar::Client::Render;

namespace RenderStar::Client::Core
{
    void ClientLifecycleModule::OnInitialize(Common::Module::ModuleContext& context)
    {
        SetupGameplayLogic(context);
        SetupMainLoop();

        logger->info("ClientLifecycleModule initialized");
    }

    void ClientLifecycleModule::OnCleanup()
    {
        logger->info("ClientLifecycleModule cleaning up render resources...");

        uniformPool.clear();
        cachedBufferManager = nullptr;
        cachedUniformManager = nullptr;
        testMesh.reset();
        testShader.reset();

        logger->info("ClientLifecycleModule cleanup complete");
    }

    Common::Event::EventResult ClientLifecycleModule::OnRenderInitializeEvent(Common::Module::ModuleContext& context, IRenderBackend* backend)
    {
        logger->info("OnRenderInitializeEvent called, backend={}", static_cast<void*>(backend));

        const auto assetModule = context.GetModule<Common::Asset::AssetModule>();

        if (!assetModule.has_value())
            return Common::Event::EventResult::Failure("AssetModule not found");

        IShaderManager* shaderManager = backend->GetShaderManager();
        IBufferManager* bufferManager = backend->GetBufferManager();
        IUniformManager* uniformManager = backend->GetUniformManager();

        if (!shaderManager || !bufferManager || !uniformManager)
            return Common::Event::EventResult::Failure("Failed to get managers from renderer backent");

        const auto vertexAsset = assetModule->get().LoadText(Common::Asset::AssetLocation::Parse("renderstar:shader/test.vert"));
        const auto fragmentAsset = assetModule->get().LoadText(Common::Asset::AssetLocation::Parse("renderstar:shader/test.frag"));

        if (!vertexAsset.IsValid() || !fragmentAsset.IsValid())
            return Common::Event::EventResult::Failure("Failed to load shader assets");

        testShader = shaderManager->CreateFromTextAssets(*vertexAsset.Get(), *fragmentAsset.Get());

        if (!testShader || !testShader->IsValid())
            return Common::Event::EventResult::Failure("Failed to create test shader");

        const std::vector vertices =
        {
            Vertex(-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f),
            Vertex( 0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f),
            Vertex( 0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f),
            Vertex(-0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f),
            Vertex(-0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f),
            Vertex( 0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f),
            Vertex( 0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f),
            Vertex(-0.5f,  0.5f,  0.5f, 0.5f, 0.5f, 0.5f, 0.0f, 1.0f)
        };

        const std::vector<uint32_t> indices =
        {
            0, 2, 1, 0, 3, 2,
            4, 5, 6, 4, 6, 7,
            0, 7, 4, 0, 3, 7,
            1, 2, 6, 1, 6, 5,
            3, 6, 2, 3, 7, 6,
            0, 1, 5, 0, 5, 4
        };

        testMesh = std::make_unique<Resource::Mesh>(*bufferManager, Vertex::LAYOUT, PrimitiveType::TRIANGLES);
        testMesh->SetVertices(vertices);
        testMesh->SetIndices(indices);

        cachedBufferManager = bufferManager;
        cachedUniformManager = uniformManager;

        if (auto cameraAffector = context.GetModule<Render::Affectors::CameraAffector>(); cameraAffector.has_value())
            cameraAffector->get().SetViewportSize(backend->GetWidth(), backend->GetHeight());

        logger->info("Test geometry initialized successfully");

        return Common::Event::EventResult::Success();
    }

    Common::Event::EventResult ClientLifecycleModule::OnRenderFrameEvent(IRenderBackend* backend)
    {
        if (backend == nullptr || !backend->IsInitialized())
            return Common::Event::EventResult::Failure("Renderer backend was not in a proper state");

        if (!cachedBufferManager || !cachedUniformManager)
            return Common::Event::EventResult::Failure("Renderer managers were not initialized");

        backend->BeginFrame();
        uniformPoolIndex = 0;

        testRotationAngle += 0.5f;

        if (testRotationAngle >= 360.0f)
            testRotationAngle -= 360.0f;

        glm::mat4 viewProjection(1.0f);

        if (const auto componentModule = context->GetModule<Common::Component::ComponentModule>(); componentModule.has_value() && playerEntity.IsValid())
        {
            if (auto cameraOpt = componentModule->get().GetComponent<Components::Camera>(playerEntity); cameraOpt.has_value())
                viewProjection = cameraOpt->get().GetViewProjectionMatrix();
        }

        auto model = glm::mat4(1.0f);

        model = glm::rotate(model, glm::radians(testRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(testRotationAngle * 0.5f), glm::vec3(1.0f, 0.0f, 0.0f));

        const StandardUniforms uniformData(model, viewProjection, glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));
        const int32_t frameIndex = backend->GetCurrentFrame();

        auto& cubeSlot = AcquireUniformSlot();
        cubeSlot.buffer->SetSubData(&uniformData, StandardUniforms::Size(), 0);
        backend->SubmitDrawCommand(testShader.get(), cubeSlot.binding.get(), frameIndex, testMesh->GetUnderlyingMesh());
        backend->ExecuteDrawCommands();

        if (auto playerModuleOpt = context->GetModule<Gameplay::ClientPlayerModule>(); playerModuleOpt.has_value())
        {
            auto& playerModule = playerModuleOpt->get();

            if (const auto componentModule = context->GetModule<Common::Component::ComponentModule>(); componentModule.has_value() && playerEntity.IsValid())
            {
                if (auto transformOpt = componentModule->get().GetComponent<Common::Component::Transform>(playerEntity); transformOpt.has_value())
                {
                    float yaw = -90.0f;
                    float pitch = 0.0f;

                    if (auto controllerOpt = componentModule->get().GetComponent<Gameplay::PlayerController>(playerEntity); controllerOpt.has_value())
                    {
                        yaw = controllerOpt->get().yaw;
                        pitch = controllerOpt->get().pitch;
                    }

                    const auto& pos = transformOpt->get().position;
                    playerModule.SendLocalPosition(pos.x, pos.y, pos.z, yaw, pitch);
                }
            }

            for (const auto& [id, remote] : playerModule.GetRemotePlayers())
            {
                auto remoteModel = glm::translate(glm::mat4(1.0f), remote.position);
                remoteModel = glm::rotate(remoteModel, glm::radians(remote.yaw + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

                const StandardUniforms remoteUniforms(remoteModel, viewProjection, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

                auto& remoteSlot = AcquireUniformSlot();
                remoteSlot.buffer->SetSubData(&remoteUniforms, StandardUniforms::Size(), 0);
                backend->SubmitDrawCommand(testShader.get(), remoteSlot.binding.get(), frameIndex, testMesh->GetUnderlyingMesh());
                backend->ExecuteDrawCommands();
            }
        }

        backend->EndFrame();

        return Common::Event::EventResult::Success();
    }

    UniformSlot& ClientLifecycleModule::AcquireUniformSlot()
    {
        if (uniformPoolIndex < uniformPool.size())
            return uniformPool[uniformPoolIndex++];

        UniformSlot slot;
        slot.buffer = cachedBufferManager->CreateUniformBuffer(StandardUniforms::Size());
        slot.binding = cachedUniformManager->CreateBindingForShader(testShader.get());

        if (slot.binding)
            slot.binding->UpdateBuffer(0, slot.buffer.get(), StandardUniforms::Size());

        uniformPool.push_back(std::move(slot));
        return uniformPool[uniformPoolIndex++];
    }

    void ClientLifecycleModule::SetupGameplayLogic(Common::Module::ModuleContext& context)
    {
        if (const auto coreEventBus = context.GetEventBus<Event::ClientCoreEventBus>(); !coreEventBus.has_value())
        {
            logger->error("ClientCoreEventBus not found");
            return;
        }

        const auto renderEventBus = context.GetEventBus<Event::ClientRenderEventBus>();

        if (!renderEventBus.has_value())
        {
            logger->error("ClientRenderEventBus not found");
            return;
        }

        logger->info("Subscribing to ClientRendererInitializedEvent");

        renderEventBus.value().get().Subscribe<Event::Events::ClientRendererInitializedEvent>([this](const auto& event)
        {
            return OnRenderInitializeEvent(*this->context, event.backend);
        });

        renderEventBus.value().get().Subscribe<Event::Events::ClientRenderFrameEvent>([this](const auto& event)
        {
            return OnRenderFrameEvent(event.backend);
        });

        logger->info("Event subscriptions set up");

        const auto componentModule = context.GetModule<Common::Component::ComponentModule>();
        const auto windowModule = context.GetModule<ClientWindowModule>();

        if (!componentModule.has_value())
        {
            logger->error("ComponentModule not found");
            return;
        }

        componentModule->get().RegisterSubModule(std::make_unique<Gameplay::PlayerControllerAffector>());
        componentModule->get().RegisterSubModule(std::make_unique<Common::Component::Affectors::TransformAffector>());
        componentModule->get().RegisterSubModule(std::make_unique<Render::Affectors::CameraAffector>());

        if (windowModule.has_value())
        {
            if (auto cameraAffector = context.GetModule<Render::Affectors::CameraAffector>(); cameraAffector.has_value())
                cameraAffector->get().SetViewportSize(static_cast<int32_t>(windowModule->get().GetWidth()), static_cast<int32_t>(windowModule->get().GetHeight()));
        }

        playerEntity = componentModule->get().CreateEntity("Player");

        auto& transform = componentModule->get().AddComponent<Common::Component::Transform>(playerEntity);
        transform.position = glm::vec3(0.0f, 2.0f, 5.0f);

        auto& camera = componentModule->get().AddComponent<Components::Camera>(playerEntity);
        camera.projectionType = Components::ProjectionType::PERSPECTIVE;
        camera.fieldOfView = 60.0f;
        camera.nearPlane = 0.1f;
        camera.farPlane = 100.0f;

        componentModule->get().AddComponent<Gameplay::PlayerController>(playerEntity);

        logger->info("Player entity created and affectors registered");
    }

    void ClientLifecycleModule::SetupMainLoop() const
    {
        const auto coreEventBus = context->GetEventBus<Event::ClientCoreEventBus>();

        if (!coreEventBus.has_value())
        {
            logger->error("ClientCoreEventBus not found");
            return;
        }

        const auto renderEventBus = context->GetEventBus<Event::ClientRenderEventBus>();

        if (!renderEventBus.has_value())
        {
            logger->error("ClientRenderEventBus not found");
            return;
        }

        logger->info("Setting up tick handler on ClientCoreEventBus");

        const auto windowModule = context->GetModule<ClientWindowModule>();
        const auto inputModule = context->GetModule<Input::ClientInputModule>();
        const auto timeModule = context->GetModule<Common::Time::TimeModule>();
        const auto componentModule = context->GetModule<Common::Component::ComponentModule>();
        const auto rendererModule = context->GetModule<RendererModule>();

        if (!windowModule.has_value())
        {
            logger->error("WindowModule not found");
            return;
        }

        if (!rendererModule.has_value())
        {
            logger->error("RendererModule not found");
            return;
        }

        coreEventBus->get().SetTickHandler([=]
        {
            windowModule->get().Tick();

            if (inputModule.has_value())
                inputModule->get().Tick();

            if (windowModule->get().ShouldClose())
            {
                if (coreEventBus.has_value())
                    coreEventBus->get().Shutdown();

                return;
            }

            if (timeModule.has_value())
                timeModule->get().Tick();

            if (componentModule.has_value())
                componentModule->get().RunAffectors();

            renderEventBus.value().get().Publish(Event::Events::ClientRenderFrameEvent(rendererModule.value().get().GetBackend()));

            if (inputModule.has_value())
                inputModule->get().EndFrame();
        });
    }

    std::vector<std::type_index> ClientLifecycleModule::GetDependencies() const
    {
        return DependsOn<
            Render::RendererModule,
            Common::Component::ComponentModule,
            ClientWindowModule,
            Input::ClientInputModule,
            Common::Time::TimeModule,
            Gameplay::ClientPlayerModule>();
    }
}
