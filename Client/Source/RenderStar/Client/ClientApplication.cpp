#include "RenderStar/Client/ClientApplication.hpp"

#include "RenderStar/Client/Core/ClientLifecycleModule.hpp"
#include "RenderStar/Client/Core/ClientWindowModule.hpp"
#include "RenderStar/Client/Event/Buses/ClientCoreEventBus.hpp"
#include "RenderStar/Client/Event/Buses/ClientRenderEventBus.hpp"
#include "RenderStar/Client/Event/Events/ClientEvents.hpp"
#include "RenderStar/Client/Gameplay/ClientPlayerModule.hpp"
#include "RenderStar/Client/Gameplay/PlayerControllerAffector.hpp"
#include "RenderStar/Client/Input/ClientInputModule.hpp"
#include "RenderStar/Client/Network/ClientNetworkModule.hpp"
#include "RenderStar/Client/Render/Affectors/CameraAffector.hpp"
#include "RenderStar/Client/Render/Affectors/MapGeometryRenderAffector.hpp"
#include "RenderStar/Client/Render/Affectors/PlayerRenderAffector.hpp"
#include "RenderStar/Client/Render/Affectors/AdaptiveVolumeAffector.hpp"
#include "RenderStar/Client/Render/Affectors/SkyboxRenderAffector.hpp"
#include "RenderStar/Client/Render/Framework/RenderingFrameworkModule.hpp"
#include "RenderStar/Client/Render/Platform/RenderingPlatformModule.hpp"
#include "RenderStar/Client/Render/RendererModule.hpp"
#include "RenderStar/Common/Asset/AssetModule.hpp"
#include "RenderStar/Common/Component/Affectors/TransformAffector.hpp"
#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Configuration/ConfigurationModule.hpp"
#include "RenderStar/Common/Module/ModuleManager.hpp"
#include "RenderStar/Common/Network/PacketModule.hpp"
#include "RenderStar/Client/Core/ClientSceneModule.hpp"
#include "RenderStar/Common/Physics/PhysicsModule.hpp"
#include "RenderStar/Common/Scene/SceneModule.hpp"
#include "RenderStar/Common/Time/TimeModule.hpp"
#include "RenderStar/Client/Render/Affectors/PlayerPhysicsAffector.hpp"
#include "RenderStar/Client/Render/Affectors/RemotePlayerInterpolationAffector.hpp"
#include "RenderStar/Client/UI/UIStackModule.hpp"

#include <spdlog/spdlog.h>

#include <filesystem>

namespace RenderStar::Client
{
    ClientApplication::ClientApplication() = default;

    ClientApplication::~ClientApplication() = default;

    int ClientApplication::Run(int argumentCount, char* argumentValues[])
    {
        spdlog::set_level(spdlog::level::info);
        spdlog::info("RenderStar Client starting...");

        const std::filesystem::path executablePath(argumentValues[0]);
        resourceBasePath = executablePath.parent_path();
        spdlog::info("Resource base path set to: {}", resourceBasePath.string());

        Initialize();

        moduleManager->Start();
        moduleManager->RunMainLoop();

        Shutdown();

        spdlog::info("RenderStar Client shut down successfully");
        return 0;
    }

    void ClientApplication::Initialize()
    {
        moduleManager = Common::Module::ModuleManager::Builder()
            .EventBus(std::make_unique<Event::ClientCoreEventBus>())
            .EventBus(std::make_unique<Event::ClientRenderEventBus>())
            .Module(std::make_unique<Common::Configuration::ConfigurationModule>(resourceBasePath))
            .Module(std::make_unique<Common::Asset::AssetModule>(resourceBasePath))
            .Module(std::make_unique<Common::Time::TimeModule>())
            .Module(std::make_unique<Common::Physics::PhysicsModule>())
            .Module(Common::Component::ComponentModule::Builder()
                .Affector(std::make_unique<Gameplay::PlayerControllerAffector>())
                .Affector(std::make_unique<Render::Affectors::PlayerPhysicsAffector>())
                .Affector(std::make_unique<Render::Affectors::RemotePlayerInterpolationAffector>())
                .Affector(std::make_unique<Common::Component::Affectors::TransformAffector>())
                .Affector(std::make_unique<Render::Affectors::CameraAffector>())
                .Affector(std::make_unique<Render::Affectors::AdaptiveVolumeAffector>())
                .Affector(std::make_unique<Render::Affectors::SkyboxRenderAffector>())
                .Affector(std::make_unique<Render::Affectors::MapGeometryRenderAffector>())
                .Affector(std::make_unique<Render::Affectors::PlayerRenderAffector>())
                .Build())
            .Module(std::make_unique<Common::Scene::SceneModule>())
            .Module(std::make_unique<Core::ClientWindowModule>())
            .Module(std::make_unique<Input::ClientInputModule>())
            .Module(std::make_unique<Render::RendererModule>())
            .Module(std::make_unique<Render::Framework::RenderingFrameworkModule>())
            .Module(std::make_unique<Render::Platform::RenderingPlatformModule>())
            .Module(std::make_unique<Common::Network::PacketModule>())
            .Module(std::make_unique<Network::ClientNetworkModule>())
            .Module(std::make_unique<Core::ClientSceneModule>())
            .Module(std::make_unique<Gameplay::ClientPlayerModule>())
            .Module(std::make_unique<UI::UIStackModule>())
            .Module(std::make_unique<Core::ClientLifecycleModule>())
            .Build();

        moduleManager->GetContext().GetEventBus<Event::ClientCoreEventBus>().value().get().Publish(Event::Events::ClientPreinitializationEvent());
        moduleManager->GetContext().GetEventBus<Event::ClientCoreEventBus>().value().get().Publish(Event::Events::ClientInitializationEvent());
    }

    void ClientApplication::Shutdown()
    {
        moduleManager.reset();
    }
}
