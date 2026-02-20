#include "RenderStar/Client/ClientApplication.hpp"

#include "RenderStar/Client/Core/ClientLifecycleModule.hpp"
#include "RenderStar/Client/Core/ClientSceneModule.hpp"
#include "RenderStar/Client/Core/ClientWindowModule.hpp"
#include "RenderStar/Client/Event/Buses/ClientCoreEventBus.hpp"
#include "RenderStar/Client/Event/Buses/ClientRenderEventBus.hpp"
#include "RenderStar/Client/Event/Events/ClientEvents.hpp"
#include "RenderStar/Client/Gameplay/ClientPlayerModule.hpp"
#include "RenderStar/Client/Input/ClientInputModule.hpp"
#include "RenderStar/Client/Network/ClientNetworkModule.hpp"
#include "RenderStar/Client/Render/RendererModule.hpp"
#include "RenderStar/Common/Asset/AssetModule.hpp"
#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Configuration/ConfigurationModule.hpp"
#include "RenderStar/Common/Module/ModuleManager.hpp"
#include "RenderStar/Common/Scene/SceneModule.hpp"
#include "RenderStar/Common/Time/TimeModule.hpp"

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
            .Module(std::make_unique<Common::Component::ComponentModule>())
            .Module(std::make_unique<Common::Scene::SceneModule>())
            .Module(std::make_unique<Core::ClientWindowModule>())
            .Module(std::make_unique<Input::ClientInputModule>())
            .Module(std::make_unique<Render::RendererModule>())
            .Module(std::make_unique<Network::ClientNetworkModule>())
            .Module(std::make_unique<Gameplay::ClientPlayerModule>())
            .Module(std::make_unique<Core::ClientSceneModule>())
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
