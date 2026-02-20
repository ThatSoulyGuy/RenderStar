#include "RenderStar/Server/Core/ServerSceneModule.hpp"
#include "RenderStar/Common/Asset/AssetModule.hpp"
#include "RenderStar/Common/Asset/AssetLocation.hpp"
#include "RenderStar/Common/Asset/IBinaryAsset.hpp"
#include "RenderStar/Common/Configuration/ConfigurationModule.hpp"
#include "RenderStar/Common/Event/EventResult.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"
#include "RenderStar/Common/Network/Packets/SceneDataPacket.hpp"
#include "RenderStar/Common/Scene/MapbinLoader.hpp"
#include "RenderStar/Common/Scene/SceneModule.hpp"
#include "RenderStar/Server/Event/Buses/ServerCoreEventBus.hpp"
#include "RenderStar/Server/Event/Events/ClientJoinedEvent.hpp"
#include "RenderStar/Server/Network/ServerNetworkModule.hpp"

namespace RenderStar::Server::Core
{
    void ServerSceneModule::OnInitialize(Common::Module::ModuleContext& context)
    {
        auto networkModuleOpt = context.GetModule<Network::ServerNetworkModule>();

        if (!networkModuleOpt.has_value())
        {
            logger->error("ServerNetworkModule not found");
            return;
        }

        networkModule = &networkModuleOpt->get();

        auto configModuleOpt = context.GetModule<Common::Configuration::ConfigurationModule>();

        if (!configModuleOpt.has_value())
        {
            logger->error("ConfigurationModule not found");
            return;
        }

        std::string sceneFile;

        if (auto configOpt = configModuleOpt->get().For<ServerSceneModule>("render_star", "server_settings.xml"))
        {
            if (auto sceneOpt = (*configOpt)->GetString("scene_file"))
                sceneFile = *sceneOpt;
        }

        if (sceneFile.empty())
        {
            logger->info("No scene_file configured, skipping scene load");
        }
        else
        {
            auto assetModuleOpt = context.GetModule<Common::Asset::AssetModule>();

            if (!assetModuleOpt.has_value())
            {
                logger->error("AssetModule not found");
                return;
            }

            auto binaryAsset = assetModuleOpt->get().LoadBinary(Common::Asset::AssetLocation::Parse(sceneFile));

            if (!binaryAsset.IsValid())
            {
                logger->error("Failed to load scene file: {}", sceneFile);
                return;
            }

            auto scene = Common::Scene::MapbinLoader::Load(binaryAsset.Get()->GetDataView());

            if (!scene.has_value())
            {
                logger->error("Failed to parse mapbin file: {}", sceneFile);
                return;
            }

            auto sceneModuleOpt = context.GetModule<Common::Scene::SceneModule>();

            if (sceneModuleOpt.has_value())
                sceneModuleOpt->get().SetMapGeometry(std::move(scene.value()));

            size_t totalVertices = 0;

            if (sceneModuleOpt.has_value() && sceneModuleOpt->get().HasMapGeometry())
            {
                for (const auto& group : sceneModuleOpt->get().GetMapGeometry()->groups)
                    totalVertices += group.vertexCount;
            }

            logger->info("Loaded scene '{}': {} groups, {} vertices", sceneFile,
                sceneModuleOpt.has_value() ? sceneModuleOpt->get().GetMapGeometry()->groups.size() : 0,
                totalVertices);
        }

        auto eventBus = context.GetEventBus<Event::Buses::ServerCoreEventBus>();

        if (eventBus.has_value())
        {
            eventBus->get().Subscribe<Event::Events::ClientJoinedEvent>(
                [this](const Event::Events::ClientJoinedEvent& event)
                {
                    OnClientJoined(event.connection);
                    return Common::Event::EventResult::Success();
                });
        }

        logger->info("ServerSceneModule initialized");
    }

    void ServerSceneModule::OnClientJoined(std::shared_ptr<Network::ClientConnection> connection)
    {
        auto sceneModuleOpt = context->GetModule<Common::Scene::SceneModule>();

        if (!sceneModuleOpt.has_value() || !sceneModuleOpt->get().HasMapGeometry())
            return;

        const auto& mapGeometry = sceneModuleOpt->get().GetMapGeometry();

        Common::Network::Packets::SceneDataPacket packet;
        packet.groups = mapGeometry->groups;

        networkModule->Send(*connection, packet);

        logger->info("Sent scene data ({} groups) to {}", packet.groups.size(), connection->remoteAddress);
    }

    std::vector<std::type_index> ServerSceneModule::GetDependencies() const
    {
        return DependsOn<Network::ServerNetworkModule>();
    }
}
