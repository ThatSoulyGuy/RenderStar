#include "RenderStar/Server/Core/ServerSceneModule.hpp"
#include "RenderStar/Common/Component/Components/MapGeometry.hpp"
#include "RenderStar/Common/Event/EventResult.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"
#include "RenderStar/Common/Network/Packets/EntityBatchPacket.hpp"
#include "RenderStar/Common/Network/Packets/EntityCreatePacket.hpp"
#include "RenderStar/Common/Network/Packets/EntityDestroyPacket.hpp"
#include "RenderStar/Common/Scene/SceneModule.hpp"
#include "RenderStar/Server/Event/Buses/ServerCoreEventBus.hpp"
#include "RenderStar/Server/Event/Events/ClientJoinedEvent.hpp"
#include "RenderStar/Server/Network/ServerNetworkModule.hpp"

namespace RenderStar::Server::Core
{
    void ServerSceneModule::OnInitialize(Common::Module::ModuleContext& context)
    {
        networkModule = &context.GetDependency<Network::ServerNetworkModule>();
        sceneModule = &context.GetDependency<Common::Scene::SceneModule>();

        sceneModule->RegisterSerializableComponent<Common::Component::MapGeometry>();

        auto eventBus = context.GetEventBus<Event::Buses::ServerCoreEventBus>();

        if (eventBus.has_value())
        {
            eventBus->get().Subscribe<Event::Events::ClientJoinedEvent>(
                [this](const Event::Events::ClientJoinedEvent& event)
                {
                    OnClientJoined(event.connection);
                    return Common::Event::EventResult::Success();
                }, Common::Event::HandlerPriority::HIGH);
        }

        logger->info("ServerSceneModule initialized");
    }

    void ServerSceneModule::OnClientJoined(std::shared_ptr<Network::ClientConnection> connection)
    {
        SendEntityBatches(*connection);
    }

    void ServerSceneModule::SendEntityBatches(Network::ClientConnection& connection)
    {
        auto allIds = sceneModule->GetOwnedEntityIds();

        if (allIds.empty())
        {
            logger->info("No scene entities to send to {}", connection.remoteAddress);
            return;
        }

        const auto totalEntities = static_cast<int32_t>(allIds.size());
        const int32_t totalBatches = (totalEntities + DEFAULT_BATCH_SIZE - 1) / DEFAULT_BATCH_SIZE;

        for (int32_t batchIdx = 0; batchIdx < totalBatches; ++batchIdx)
        {
            const int32_t startOffset = batchIdx * DEFAULT_BATCH_SIZE;
            const int32_t endOffset = std::min(startOffset + DEFAULT_BATCH_SIZE, totalEntities);

            std::vector<int32_t> batchIds(allIds.begin() + startOffset, allIds.begin() + endOffset);

            Common::Network::Packets::EntityBatchPacket packet;
            packet.batchIndex = batchIdx;
            packet.totalBatches = totalBatches;
            packet.xmlData = sceneModule->SerializeEntities(batchIds);

            logger->info("Batch {}/{}: {} entities, xmlData size={}", batchIdx + 1, totalBatches, batchIds.size(), packet.xmlData.size());
            logger->debug("Batch XML: {}", packet.xmlData);

            networkModule->Send(connection, packet);
        }

        logger->info("Sent {} entities in {} batches to {}", totalEntities, totalBatches, connection.remoteAddress);
    }

    Common::Component::GameObject ServerSceneModule::CreateAndBroadcastEntity(const std::string& name)
    {
        auto entity = sceneModule->CreateEntity(name);
        auto xmlData = sceneModule->SerializeEntities({ entity.id });

        Common::Network::Packets::EntityCreatePacket packet;
        packet.xmlData = std::move(xmlData);
        networkModule->Broadcast(packet);

        return entity;
    }

    void ServerSceneModule::DestroyAndBroadcastEntity(Common::Component::GameObject entity)
    {
        Common::Network::Packets::EntityDestroyPacket packet;
        packet.serverEntityId = entity.id;
        networkModule->Broadcast(packet);

        sceneModule->DestroyEntity(entity);
    }

    std::vector<std::type_index> ServerSceneModule::GetDependencies() const
    {
        return DependsOn<
            Network::ServerNetworkModule,
            Common::Scene::SceneModule>();
    }
}
