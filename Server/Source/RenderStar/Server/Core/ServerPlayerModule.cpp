#include "RenderStar/Server/Core/ServerPlayerModule.hpp"
#include "RenderStar/Common/Event/AbstractEventBus.hpp"
#include "RenderStar/Common/Event/EventResult.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"
#include "RenderStar/Common/Network/PacketModule.hpp"
#include "RenderStar/Common/Network/Packets/PlayerAssignPacket.hpp"
#include "RenderStar/Common/Network/Packets/PlayerSpawnPacket.hpp"
#include "RenderStar/Common/Network/Packets/PlayerDespawnPacket.hpp"
#include "RenderStar/Common/Network/Packets/PlayerPositionPacket.hpp"
#include "RenderStar/Server/Event/Buses/ServerCoreEventBus.hpp"
#include "RenderStar/Server/Event/Events/ClientJoinedEvent.hpp"
#include "RenderStar/Server/Event/Events/ClientLeftEvent.hpp"
#include "RenderStar/Server/Network/ServerNetworkModule.hpp"

namespace RenderStar::Server::Core
{
    void ServerPlayerModule::OnInitialize(Common::Module::ModuleContext& context)
    {
        auto networkModuleOpt = context.GetModule<Network::ServerNetworkModule>();

        if (!networkModuleOpt.has_value())
        {
            logger->error("ServerNetworkModule not found");
            return;
        }

        networkModule = &networkModuleOpt->get();
        packetModule = networkModule->GetPacketModule();

        if (!packetModule)
        {
            logger->error("PacketModule not found on ServerNetworkModule");
            return;
        }

        packetModule->RegisterPacket<Common::Network::Packets::PlayerAssignPacket>(1);
        packetModule->RegisterPacket<Common::Network::Packets::PlayerSpawnPacket>(2);
        packetModule->RegisterPacket<Common::Network::Packets::PlayerDespawnPacket>(3);
        packetModule->RegisterPacket<Common::Network::Packets::PlayerPositionPacket>(4);

        networkModule->SetPacketReceivedCallback(
            [this](std::shared_ptr<Network::ClientConnection> connection, Common::Network::IPacket& packet)
            {
                OnPacketReceived(std::move(connection), packet);
            });

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
        }

        logger->info("ServerPlayerModule initialized");
    }

    void ServerPlayerModule::OnClientJoined(std::shared_ptr<Network::ClientConnection> connection)
    {
        int32_t playerId = nextPlayerId++;

        PlayerState state;
        state.connection = connection;

        Common::Network::Packets::PlayerAssignPacket assignPacket;
        assignPacket.playerId = playerId;
        networkModule->Send(*connection, assignPacket);

        for (const auto& [existingId, existingState] : players)
        {
            Common::Network::Packets::PlayerSpawnPacket existingSpawn;
            existingSpawn.playerId = existingId;
            existingSpawn.x = existingState.posX;
            existingSpawn.y = existingState.posY;
            existingSpawn.z = existingState.posZ;
            existingSpawn.yaw = existingState.yaw;
            existingSpawn.pitch = existingState.pitch;
            networkModule->Send(*connection, existingSpawn);

            Common::Network::Packets::PlayerSpawnPacket newSpawn;
            newSpawn.playerId = playerId;
            newSpawn.x = state.posX;
            newSpawn.y = state.posY;
            newSpawn.z = state.posZ;
            newSpawn.yaw = state.yaw;
            newSpawn.pitch = state.pitch;
            networkModule->Send(*existingState.connection, newSpawn);
        }

        players[playerId] = std::move(state);

        logger->info("Player {} joined from {}", playerId, connection->remoteAddress);
    }

    void ServerPlayerModule::OnClientLeft(std::shared_ptr<Network::ClientConnection> connection)
    {
        int32_t leftPlayerId = -1;

        for (auto it = players.begin(); it != players.end(); ++it)
        {
            if (it->second.connection.get() == connection.get())
            {
                leftPlayerId = it->first;
                players.erase(it);
                break;
            }
        }

        if (leftPlayerId < 0)
            return;

        Common::Network::Packets::PlayerDespawnPacket despawnPacket;
        despawnPacket.playerId = leftPlayerId;

        for (const auto& [id, state] : players)
            networkModule->Send(*state.connection, despawnPacket);

        logger->info("Player {} left", leftPlayerId);
    }

    void ServerPlayerModule::OnPacketReceived(std::shared_ptr<Network::ClientConnection> connection, Common::Network::IPacket& packet)
    {
        auto* posPacket = dynamic_cast<Common::Network::Packets::PlayerPositionPacket*>(&packet);

        if (!posPacket)
        {
            packetModule->HandlePacket(packet);
            return;
        }

        int32_t sourcePlayerId = -1;

        for (auto& [id, state] : players)
        {
            if (state.connection.get() == connection.get())
            {
                sourcePlayerId = id;
                state.posX = posPacket->x;
                state.posY = posPacket->y;
                state.posZ = posPacket->z;
                state.yaw = posPacket->yaw;
                state.pitch = posPacket->pitch;
                break;
            }
        }

        if (sourcePlayerId < 0)
            return;

        posPacket->playerId = sourcePlayerId;

        for (const auto& [id, state] : players)
        {
            if (id != sourcePlayerId)
                networkModule->Send(*state.connection, *posPacket);
        }
    }

    std::vector<std::type_index> ServerPlayerModule::GetDependencies() const
    {
        return DependsOn<Network::ServerNetworkModule>();
    }
}
