#include "RenderStar/Client/Core/ClientLifecycleModule.hpp"
#include "RenderStar/Client/Core/ClientSceneModule.hpp"
#include "RenderStar/Client/Core/ClientWindowModule.hpp"
#include "RenderStar/Client/Gameplay/ClientPlayerModule.hpp"
#include "RenderStar/Client/Gameplay/PlayerControllerAffector.hpp"
#include "RenderStar/Client/Input/ClientInputModule.hpp"
#include "RenderStar/Client/Event/Buses/ClientCoreEventBus.hpp"
#include "RenderStar/Client/Event/Buses/ClientRenderEventBus.hpp"
#include "RenderStar/Client/Event/Events/ClientEvents.hpp"
#include "RenderStar/Client/Render/Affectors/CameraAffector.hpp"
#include "RenderStar/Client/Render/Affectors/MapGeometryRenderAffector.hpp"
#include "RenderStar/Client/Render/Affectors/PlayerRenderAffector.hpp"
#include "RenderStar/Client/Render/Backend/IRenderBackend.hpp"
#include "RenderStar/Client/Render/Components/Camera.hpp"
#include "RenderStar/Client/Render/Components/Light.hpp"
#include "RenderStar/Client/Render/Framework/RenderingFrameworkModule.hpp"
#include "RenderStar/Client/Render/Platform/RenderingPlatformModule.hpp"
#include "RenderStar/Client/Render/RendererModule.hpp"
#include "RenderStar/Client/Render/Resource/IBufferManager.hpp"
#include "RenderStar/Client/Render/Resource/IShaderManager.hpp"
#include "RenderStar/Client/Render/Resource/ITextureManager.hpp"
#include "RenderStar/Client/Render/Resource/IUniformManager.hpp"
#include "RenderStar/Client/Render/Resource/StandardUniforms.hpp"
#include "RenderStar/Client/Render/Shader/RsslCompiler.hpp"
#include "RenderStar/Client/Render/Framework/LitVertex.hpp"
#include "RenderStar/Client/Network/ClientNetworkModule.hpp"
#include "RenderStar/Common/Asset/AssetModule.hpp"
#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Component/Components/Transform.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"
#include "RenderStar/Common/Scene/SceneModule.hpp"
#include "RenderStar/Common/Time/TimeModule.hpp"

using namespace RenderStar::Client::Render;

namespace RenderStar::Client::Core
{
    void ClientLifecycleModule::OnInitialize(Common::Module::ModuleContext& context)
    {
        if (auto platformOpt = context.GetModule<Render::Platform::RenderingPlatformModule>(); platformOpt.has_value())
        {
            platformModule = &platformOpt->get();
            logger->info("RenderingPlatformModule detected, enabled={}", platformModule->IsEnabled());
        }

        if (auto frameworkOpt = context.GetModule<Render::Framework::RenderingFrameworkModule>(); frameworkOpt.has_value())
            frameworkModule = &frameworkOpt->get();

        SetupGameplayLogic(context);
        SetupMainLoop();

        auto& networkModule = context.GetDependency<Network::ClientNetworkModule>();
        networkModule.Connect("127.0.0.1", 25565);

        logger->info("ClientLifecycleModule initialized");
    }

    void ClientLifecycleModule::OnCleanup()
    {
        logger->info("ClientLifecycleModule cleaning up...");

        if (auto sceneModule = context->GetModule<Common::Scene::SceneModule>(); sceneModule.has_value())
            sceneModule->get().ClearScene();

        if (frameworkModule)
        {
            frameworkModule->Cleanup();
            frameworkModule = nullptr;
        }

        uniformPool.clear();
        cachedBufferManager = nullptr;
        cachedUniformManager = nullptr;
        cachedTextureManager = nullptr;

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

        const auto rsslAsset = assetModule->get().LoadText(
            Common::Asset::AssetLocation::Parse("renderstar:shader/scene_geometry.rssl"));

        if (!rsslAsset.IsValid())
            return Common::Event::EventResult::Failure("Failed to load scene_geometry.rssl asset");

        auto compiled = Render::Shader::RsslCompiler::Compile(rsslAsset.Get()->GetContent());

        if (!compiled.IsValid())
            return Common::Event::EventResult::Failure("Failed to compile scene_geometry.rssl");

        cachedBufferManager = bufferManager;
        cachedUniformManager = uniformManager;
        cachedTextureManager = backend->GetTextureManager();

        if (auto cameraAffector = context.GetModule<Render::Affectors::CameraAffector>(); cameraAffector.has_value())
            cameraAffector->get().SetViewportSize(backend->GetWidth(), backend->GetHeight());

        auto shader = shaderManager->CreateFromSource(
            Render::ShaderSource{compiled.vertexGlsl, compiled.fragmentGlsl, {}});

        if (!shader || !shader->IsValid())
            return Common::Event::EventResult::Failure("Failed to create shader program");

        if (frameworkModule)
            frameworkModule->SetupRenderState(bufferManager);

        Render::IBufferHandle* sceneLightingBuffer = frameworkModule ? frameworkModule->GetSceneLightingBuffer() : nullptr;

        if (auto mapGeometryAffector = context.GetModule<Render::Affectors::MapGeometryRenderAffector>(); mapGeometryAffector.has_value())
        {
            mapGeometryAffector->get().SetupRenderState(bufferManager, uniformManager, cachedTextureManager);
            mapGeometryAffector->get().SetSceneLightingBuffer(sceneLightingBuffer);
            mapGeometryAffector->get().SetShader(std::move(shader));

            const auto shadowRsslAsset = assetModule->get().LoadText(
                Common::Asset::AssetLocation::Parse("renderstar:shader/shadow_depth.rssl"));

            if (shadowRsslAsset.IsValid() && platformModule && platformModule->IsEnabled())
            {
                auto shadowCompiled = Render::Shader::RsslCompiler::Compile(shadowRsslAsset.Get()->GetContent());

                if (shadowCompiled.IsValid())
                {
                    auto shadowShader = platformModule->CompileShaderForTarget(
                        shadowCompiled.vertexGlsl, shadowCompiled.fragmentGlsl,
                        "shadow_map", Render::Framework::LitVertex::LAYOUT);

                    if (shadowShader && shadowShader->IsValid())
                        mapGeometryAffector->get().SetShadowShader(std::move(shadowShader));
                }
            }
        }

        if (auto playerRenderAffector = context.GetModule<Render::Affectors::PlayerRenderAffector>(); playerRenderAffector.has_value())
        {
            playerRenderAffector->get().SetupRenderState(bufferManager, uniformManager, cachedTextureManager);
            playerRenderAffector->get().SetSceneLightingBuffer(sceneLightingBuffer);

            auto playerShader = shaderManager->CreateFromSource(
                Render::ShaderSource{compiled.vertexGlsl, compiled.fragmentGlsl, {}});

            if (playerShader && playerShader->IsValid())
                playerRenderAffector->get().SetShader(std::move(playerShader));
        }

        auto& componentModule = context.GetDependency<Common::Component::ComponentModule>();

        if (auto sceneModule = context.GetModule<Common::Scene::SceneModule>(); sceneModule.has_value())
        {
            auto lightEntity = sceneModule->get().CreateEntity();
            componentModule.AddComponent<Render::Components::Light>(lightEntity,
                Render::Components::Light::Directional(
                    glm::normalize(glm::vec3(-0.5f, -1.0f, -0.3f)),
                    glm::vec3(1.0f), 1.0f));
        }

        logger->info("Render state initialized successfully");

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

        auto& componentModule = context->GetDependency<Common::Component::ComponentModule>();
        auto& playerModule = context->GetDependency<Gameplay::ClientPlayerModule>();
        auto mapGeometryAffectorOpt = context->GetModule<Render::Affectors::MapGeometryRenderAffector>();

        if (mapGeometryAffectorOpt.has_value())
            mapGeometryAffectorOpt->get().CheckForNewMapGeometry(componentModule);

        testRotationAngle += 0.5f;

        if (testRotationAngle >= 360.0f)
            testRotationAngle -= 360.0f;

        glm::mat4 viewProjection(1.0f);

        playerEntity = playerModule.GetLocalPlayerEntity();
        glm::vec3 cameraPosition(0.0f);

        if (playerEntity.IsValid())
        {
            if (auto cameraOpt = componentModule.GetComponent<Components::Camera>(playerEntity); cameraOpt.has_value())
                viewProjection = cameraOpt->get().GetViewProjectionMatrix();

            if (auto transformOpt = componentModule.GetComponent<Common::Component::Transform>(playerEntity); transformOpt.has_value())
                cameraPosition = transformOpt->get().worldPosition;
        }

        if (frameworkModule)
            frameworkModule->CollectSceneData(componentModule, cameraPosition);

        auto model = glm::mat4(1.0f);

        model = glm::rotate(model, glm::radians(testRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(testRotationAngle * 0.5f), glm::vec3(1.0f, 0.0f, 0.0f));

        const StandardUniforms uniformData(model, viewProjection, glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));

        auto& [cubeBuffer, cubeBinding] = AcquireUniformSlot();

        cubeBuffer->SetSubData(&uniformData, StandardUniforms::Size(), 0);
        backend->ExecuteDrawCommands();

        if (platformModule && platformModule->IsEnabled())
        {
            if (frameworkModule && mapGeometryAffectorOpt.has_value())
            {
                platformModule->BeginTarget("shadow_map", true);
                mapGeometryAffectorOpt->get().RenderShadowDepth(componentModule, backend, frameworkModule->GetLightViewProjection());
                backend->ExecuteDrawCommands();
                platformModule->EndTarget("shadow_map");

                auto* shadowTex = platformModule->GetTargetColorTexture("shadow_map");
                mapGeometryAffectorOpt->get().SetShadowMapTexture(shadowTex);
            }

            if (mapGeometryAffectorOpt.has_value())
                mapGeometryAffectorOpt->get().Render(componentModule, backend, viewProjection);

            if (auto playerRenderAffectorOpt = context->GetModule<Render::Affectors::PlayerRenderAffector>(); playerRenderAffectorOpt.has_value())
                playerRenderAffectorOpt->get().Render(componentModule, backend, viewProjection, playerModule.GetLocalPlayerId());

            platformModule->Execute(backend);
        }
        else
        {
            if (mapGeometryAffectorOpt.has_value())
                mapGeometryAffectorOpt->get().Render(componentModule, backend, viewProjection);

            if (auto playerRenderAffectorOpt = context->GetModule<Render::Affectors::PlayerRenderAffector>(); playerRenderAffectorOpt.has_value())
                playerRenderAffectorOpt->get().Render(componentModule, backend, viewProjection, playerModule.GetLocalPlayerId());

            backend->ExecuteDrawCommands();
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

        if (slot.binding)
        {
            slot.binding->UpdateBuffer(0, slot.buffer.get(), StandardUniforms::Size());

            if (cachedTextureManager)
                slot.binding->UpdateTexture(1, cachedTextureManager->GetDefaultTexture());
        }

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

        auto& windowModule = context.GetDependency<ClientWindowModule>();

        if (auto cameraAffector = context.GetModule<Render::Affectors::CameraAffector>(); cameraAffector.has_value())
            cameraAffector->get().SetViewportSize(static_cast<int32_t>(windowModule.GetWidth()), static_cast<int32_t>(windowModule.GetHeight()));

        logger->info("Gameplay logic set up, waiting for player entity from server");
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

        auto* windowModule = &context->GetDependency<ClientWindowModule>();
        auto* inputModule = &context->GetDependency<Input::ClientInputModule>();
        auto* timeModule = &context->GetDependency<Common::Time::TimeModule>();
        auto* componentModule = &context->GetDependency<Common::Component::ComponentModule>();
        auto* rendererModule = &context->GetDependency<RendererModule>();
        auto* clientSceneModule = &context->GetDependency<ClientSceneModule>();
        auto* clientPlayerModule = &context->GetDependency<Gameplay::ClientPlayerModule>();

        auto playerControllerOpt = context->GetModule<Gameplay::PlayerControllerAffector>();
        Gameplay::PlayerControllerAffector* playerControllerAffector = playerControllerOpt.has_value() ? &playerControllerOpt->get() : nullptr;

        if (!playerControllerAffector)
            logger->warn("PlayerControllerAffector not found as sub-module, authority context will not be set");
        else
            logger->info("PlayerControllerAffector found, authority context will be managed in tick loop");

        coreEventBus->get().SetTickHandler([=, &coreEventBus = coreEventBus->get(), &renderEventBus = renderEventBus->get()]
        {
            windowModule->Tick();
            inputModule->Tick();

            if (windowModule->ShouldClose())
            {
                coreEventBus.Shutdown();
                return;
            }

            timeModule->Tick();

            if (clientSceneModule->HasPendingData())
                clientSceneModule->ProcessPendingEntityData();

            clientPlayerModule->CheckForLocalPlayerEntity(*componentModule);

            if (playerControllerAffector)
            {
                int32_t playerId = clientPlayerModule->GetLocalPlayerId();

                if (playerId >= 0)
                    playerControllerAffector->SetAuthorityContext(Common::Component::AuthorityContext::AsClient(playerId));
            }

            componentModule->RunAffectors();

            clientSceneModule->SendDirtyEntityUpdates();

            renderEventBus.Publish(Event::Events::ClientRenderFrameEvent(rendererModule->GetBackend()));

            inputModule->EndFrame();
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
            Gameplay::ClientPlayerModule,
            Network::ClientNetworkModule,
            ClientSceneModule>();
    }
}
