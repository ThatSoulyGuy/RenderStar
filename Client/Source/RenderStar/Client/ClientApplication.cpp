#include "RenderStar/Client/ClientApplication.hpp"
#include "RenderStar/Client/Core/ClientWindowModule.hpp"
#include "RenderStar/Client/Core/ClientLifecycleModule.hpp"
#include "RenderStar/Client/Render/RendererModule.hpp"
#include "RenderStar/Client/Network/ClientNetworkModule.hpp"
#include "RenderStar/Client/Event/Buses/ClientCoreEventBus.hpp"
#include "RenderStar/Client/Event/Buses/ClientRenderEventBus.hpp"
#include "RenderStar/Common/Module/ModuleManager.hpp"
#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Configuration/ConfigurationFactory.hpp"
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

        std::filesystem::path executablePath(argumentValues[0]);
        std::filesystem::path basePath = executablePath.parent_path();
        Common::Configuration::ConfigurationFactory::SetResourceBasePath(basePath);
        spdlog::info("Resource base path set to: {}", basePath.string());

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
            .Module(std::make_unique<Common::Time::TimeModule>())
            .Module(std::make_unique<Common::Component::ComponentModule>())
            .Module(std::make_unique<Core::ClientWindowModule>())
            .Module(std::make_unique<Core::ClientLifecycleModule>())
            .Module(std::make_unique<Render::RendererModule>())
            .Module(std::make_unique<Network::ClientNetworkModule>())
            .Build();
    }

    void ClientApplication::Shutdown()
    {
        moduleManager.reset();
    }
}
