#include "RenderStar/Server/Core/ServerPlayerModule.hpp"
#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Component/Components/PlayerIdentity.hpp"
#include "RenderStar/Common/Component/Components/SerializableTransform.hpp"
#include "RenderStar/Common/Component/Components/Transform.hpp"
#include "RenderStar/Common/Component/EntityAuthority.hpp"
#include "RenderStar/Common/Event/AbstractEventBus.hpp"
#include "RenderStar/Common/Event/EventResult.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"
#include "RenderStar/Common/Network/Packets/ComponentUpdatePacket.hpp"
#include "RenderStar/Common/Network/Packets/EntityCreatePacket.hpp"
#include "RenderStar/Common/Network/Packets/PlayerAssignPacket.hpp"
#include "RenderStar/Common/Network/Packets/PlayerInputPacket.hpp"
#include "RenderStar/Common/Scene/SceneModule.hpp"
#include "RenderStar/Server/Core/ServerSceneModule.hpp"
#include "RenderStar/Server/Event/Buses/ServerCoreEventBus.hpp"
#include "RenderStar/Server/Event/Events/ClientJoinedEvent.hpp"
#include "RenderStar/Server/Event/Events/ClientLeftEvent.hpp"
#include "RenderStar/Server/Event/Events/PacketReceivedEvent.hpp"
#include "RenderStar/Server/Network/ServerNetworkModule.hpp"
#include "RenderStar/Server/Physics/ServerPhysicsModule.hpp"
#include <glm/glm.hpp>

namespace RenderStar::Server::Core
{
    void ServerPlayerModule::OnInitialize(Common::Module::ModuleContext& context)
    {
        networkModule = &context.GetDependency<Network::ServerNetworkModule>();
        sceneModule = &context.GetDependency<Common::Scene::SceneModule>();
        componentModule = &context.GetDependency<Common::Component::ComponentModule>();
        serverSceneModule = &context.GetDependency<ServerSceneModule>();
        serverPhysicsModule = &context.GetDependency<Physics::ServerPhysicsModule>();

        sceneModule->RegisterSerializableComponent<Common::Component::PlayerIdentity>();
        sceneModule->RegisterSerializableComponent<Common::Component::Transform>();

        auto eventBus = context.GetEventBus<Event::Buses::ServerCoreEventBus>();

        if (eventBus.has_value())
        {
            eventBus->get().Subscribe<Event::Events::ClientJoinedEvent>(
                [this](const Event::Events::ClientJoinedEvent& event)
                {
                    OnClientJoined(event.connection);
                    return Common::Event::EventResult::Success();
                });

            eventBus->get().Subscribe<Event::Events::ClientLeftEvent>(
                [this](const Event::Events::ClientLeftEvent& event)
                {
                    OnClientLeft(event.connection);
                    return Common::Event::EventResult::Success();
                });

            eventBus->get().Subscribe<Event::Events::PacketReceivedEvent>(
                [this](const Event::Events::PacketReceivedEvent& event)
                {
                    OnPacketReceived(event.connection, *event.packet);
                    return Common::Event::EventResult::Success();
                });
        }

        logger->info("ServerPlayerModule initialized");
    }

    void ServerPlayerModule::OnClientJoined(std::shared_ptr<Network::ClientConnection> connection)
    {
        int32_t playerId = nextPlayerId++;

        auto entity = sceneModule->CreateEntity("Player_" + std::to_string(playerId));

        glm::vec3 spawnPos = serverPhysicsModule->GetSpawnPosition();

        auto& transform = componentModule->AddComponent<Common::Component::Transform>(entity);
        transform.position = spawnPos;

        componentModule->AddComponent<Common::Component::PlayerIdentity>(entity,
            Common::Component::PlayerIdentity{ playerId });

        componentModule->SetEntityAuthority(entity, Common::Component::EntityAuthority::Client(playerId));

        Common::Network::Packets::PlayerAssignPacket assignPacket;
        assignPacket.playerId = playerId;
        networkModule->Send(*connection, assignPacket);

        auto xmlData = sceneModule->SerializeEntities({ entity.id });
        Common::Network::Packets::EntityCreatePacket createPacket;
        createPacket.xmlData = std::move(xmlData);
        networkModule->Broadcast(createPacket);

        PlayerState state;
        state.connection = connection;
        state.entity = entity;
        players[playerId] = std::move(state);

        serverPhysicsModule->OnPlayerJoined(playerId, entity, connection, spawnPos);

        logger->info("Player {} joined from {}, entity id={}", playerId, connection->remoteAddress, entity.id);
    }

    void ServerPlayerModule::OnClientLeft(std::shared_ptr<Network::ClientConnection> connection)
    {
        int32_t leftPlayerId = -1;

        for (auto it = players.begin(); it != players.end(); ++it)
        {
            if (it->second.connection.get() == connection.get())
            {
                leftPlayerId = it->first;
                break;
            }
        }

        if (leftPlayerId < 0)
            return;

        auto entity = players[leftPlayerId].entity;
        players.erase(leftPlayerId);

        serverPhysicsModule->OnPlayerLeft(leftPlayerId);

        if (entity.IsValid())
            serverSceneModule->DestroyAndBroadcastEntity(entity);

        logger->info("Player {} left", leftPlayerId);
    }

    void ServerPlayerModule::OnPacketReceived(std::shared_ptr<Network::ClientConnection> connection, Common::Network::IPacket& packet)
    {
        if (auto* inputPacket = dynamic_cast<Common::Network::Packets::PlayerInputPacket*>(&packet))
        {
            int32_t playerId = FindPlayerIdByConnection(*connection);

            if (playerId >= 0)
            {
                serverPhysicsModule->QueueInput(playerId, inputPacket->sequenceNumber,
                    inputPacket->inputFlags, inputPacket->yaw, inputPacket->pitch, inputPacket->deltaTime);
            }

            return;
        }

        auto* updatePacket = dynamic_cast<Common::Network::Packets::ComponentUpdatePacket*>(&packet);

        if (!updatePacket)
            return;

        int32_t playerId = FindPlayerIdByConnection(*connection);

        if (playerId < 0)
        {
            logger->warn("ComponentUpdatePacket from unknown connection");
            return;
        }

        Common::Component::GameObject entity{ updatePacket->entityId };

        if (!componentModule->CheckAuthority(entity, Common::Component::AuthorityContext::AsClient(playerId)))
        {
            logger->warn("Player {} tried to modify entity {} without authority", playerId, updatePacket->entityId);
            return;
        }

        sceneModule->UpdateEntityComponents(entity, updatePacket->xmlData);

        networkModule->Broadcast(*updatePacket, *connection);
    }

    int32_t ServerPlayerModule::FindPlayerIdByConnection(const Network::ClientConnection& connection) const
    {
        for (const auto& [id, state] : players)
        {
            if (state.connection.get() == &connection)
                return id;
        }

        return -1;
    }

    std::vector<std::type_index> ServerPlayerModule::GetDependencies() const
    {
        return DependsOn<
            Network::ServerNetworkModule,
            Common::Scene::SceneModule,
            Common::Component::ComponentModule,
            ServerSceneModule,
            Physics::ServerPhysicsModule>();
    }
}
