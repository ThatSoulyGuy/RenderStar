#include "RenderStar/Server/ServerApplication.hpp"
#include "RenderStar/Common/Module/ModuleManager.hpp"
#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Configuration/ConfigurationModule.hpp"
#include "RenderStar/Common/Asset/AssetModule.hpp"
#include "RenderStar/Server/Core/ServerLifecycleModule.hpp"
#include "RenderStar/Server/Core/ServerPlayerModule.hpp"
#include "RenderStar/Server/Event/Buses/ServerCoreEventBus.hpp"
#include "RenderStar/Server/Event/Events/ServerPreinitializationEvent.hpp"
#include "RenderStar/Server/Event/Events/ServerInitializationEvent.hpp"
#include "RenderStar/Server/Network/ServerNetworkModule.hpp"
#include <filesystem>

namespace RenderStar::Server
{
    int ServerApplication::Run(int argc, char** argv)
    {
        std::filesystem::path executablePath(argv[0]);
        std::filesystem::path basePath = executablePath.parent_path();

        auto configModule = std::make_unique<Common::Configuration::ConfigurationModule>(basePath);
        auto& configModuleRef = *configModule;

        auto manager = Common::Module::ModuleManager::Builder()
            .EventBus<Event::Buses::ServerCoreEventBus>(std::make_unique<Event::Buses::ServerCoreEventBus>())
            .Module<Common::Configuration::ConfigurationModule>(std::move(configModule))
            .Module<Common::Asset::AssetModule>(std::make_unique<Common::Asset::AssetModule>(basePath))
            .Module<Network::ServerNetworkModule>(Network::ServerNetworkModule::FromArguments(argc, argv, configModuleRef))
            .Module<Common::Component::ComponentModule>(std::make_unique<Common::Component::ComponentModule>())
            .Module<Core::ServerLifecycleModule>(std::make_unique<Core::ServerLifecycleModule>())
            .Module<Core::ServerPlayerModule>(std::make_unique<Core::ServerPlayerModule>())
            .Build();

        manager->Start();

        auto& context = manager->GetContext();
        auto eventBus = context.GetEventBus<Event::Buses::ServerCoreEventBus>();

        if (eventBus.has_value())
        {
            eventBus->get().Publish(Event::Events::ServerPreinitializationEvent{});
            eventBus->get().Publish(Event::Events::ServerInitializationEvent{});
        }

        manager->RunMainLoop();

        return 0;
    }
}
