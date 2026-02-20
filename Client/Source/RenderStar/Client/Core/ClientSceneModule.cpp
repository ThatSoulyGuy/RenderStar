#include "RenderStar/Client/Core/ClientSceneModule.hpp"
#include "RenderStar/Client/Network/ClientNetworkModule.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"
#include "RenderStar/Common/Network/PacketModule.hpp"
#include "RenderStar/Common/Network/Packets/SceneDataPacket.hpp"
#include "RenderStar/Common/Scene/SceneModule.hpp"

namespace RenderStar::Client::Core
{
    void ClientSceneModule::OnInitialize(Common::Module::ModuleContext& context)
    {
        auto networkModuleOpt = context.GetModule<Network::ClientNetworkModule>();

        if (!networkModuleOpt.has_value())
        {
            logger->error("ClientNetworkModule not found");
            return;
        }

        auto* packetModule = networkModuleOpt->get().GetPacketModule();

        if (!packetModule)
        {
            logger->error("PacketModule not found on ClientNetworkModule");
            return;
        }

        packetModule->RegisterHandler<Common::Network::Packets::SceneDataPacket>(
            [this](Common::Network::Packets::SceneDataPacket& packet)
            {
                auto sceneModuleOpt = this->context->GetModule<Common::Scene::SceneModule>();

                if (sceneModuleOpt.has_value())
                {
                    Common::Scene::MapbinScene scene;
                    scene.groups = packet.groups;
                    sceneModuleOpt->get().SetMapGeometry(std::move(scene));
                }

                size_t groupCount = packet.groups.size();

                {
                    std::lock_guard lock(sceneMutex);
                    pendingGroups = std::move(packet.groups);
                }

                hasPending.store(true);

                logger->info("Received scene data with {} groups", groupCount);
            });

        logger->info("ClientSceneModule initialized");
    }

    bool ClientSceneModule::HasPendingSceneData() const
    {
        return hasPending.load();
    }

    std::vector<Common::Scene::MapbinGroup> ClientSceneModule::TakePendingSceneData()
    {
        std::lock_guard lock(sceneMutex);
        hasPending.store(false);
        return std::move(pendingGroups);
    }

    std::vector<std::type_index> ClientSceneModule::GetDependencies() const
    {
        return DependsOn<Network::ClientNetworkModule>();
    }
}
