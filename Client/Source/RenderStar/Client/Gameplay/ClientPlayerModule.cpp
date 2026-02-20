#include "RenderStar/Client/Gameplay/ClientPlayerModule.hpp"
#include "RenderStar/Client/Network/ClientNetworkModule.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"
#include "RenderStar/Common/Network/PacketModule.hpp"
#include "RenderStar/Common/Network/Packets/PlayerAssignPacket.hpp"
#include "RenderStar/Common/Network/Packets/PlayerSpawnPacket.hpp"
#include "RenderStar/Common/Network/Packets/PlayerDespawnPacket.hpp"
#include "RenderStar/Common/Network/Packets/PlayerPositionPacket.hpp"

namespace RenderStar::Client::Gameplay
{
    void ClientPlayerModule::OnInitialize(Common::Module::ModuleContext& context)
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

        packetModule->RegisterHandler<Common::Network::Packets::PlayerAssignPacket>(
            [this](Common::Network::Packets::PlayerAssignPacket& packet)
            {
                localPlayerId.store(packet.playerId);
                logger->info("Assigned player ID: {}", packet.playerId);
            });

        packetModule->RegisterHandler<Common::Network::Packets::PlayerSpawnPacket>(
            [this](Common::Network::Packets::PlayerSpawnPacket& packet)
            {
                std::lock_guard lock(remotePlayersMutex);
                remotePlayers[packet.playerId] = RemotePlayer{
                    glm::vec3(packet.x, packet.y, packet.z),
                    packet.yaw,
                    packet.pitch
                };
                logger->info("Remote player {} spawned at ({}, {}, {})", packet.playerId, packet.x, packet.y, packet.z);
            });

        packetModule->RegisterHandler<Common::Network::Packets::PlayerDespawnPacket>(
            [this](Common::Network::Packets::PlayerDespawnPacket& packet)
            {
                std::lock_guard lock(remotePlayersMutex);
                remotePlayers.erase(packet.playerId);
                logger->info("Remote player {} despawned", packet.playerId);
            });

        packetModule->RegisterHandler<Common::Network::Packets::PlayerPositionPacket>(
            [this](Common::Network::Packets::PlayerPositionPacket& packet)
            {
                std::lock_guard lock(remotePlayersMutex);
                if (auto it = remotePlayers.find(packet.playerId); it != remotePlayers.end())
                {
                    it->second.position = glm::vec3(packet.x, packet.y, packet.z);
                    it->second.yaw = packet.yaw;
                    it->second.pitch = packet.pitch;
                }
            });

        logger->info("ClientPlayerModule initialized");
    }

    int32_t ClientPlayerModule::GetLocalPlayerId() const
    {
        return localPlayerId.load();
    }

    std::unordered_map<int32_t, RemotePlayer> ClientPlayerModule::GetRemotePlayers() const
    {
        std::lock_guard lock(remotePlayersMutex);
        return remotePlayers;
    }

    void ClientPlayerModule::SendLocalPosition(float x, float y, float z, float yaw, float pitch)
    {
        const int32_t pid = localPlayerId.load();

        if (pid < 0)
            return;

        auto networkModuleOpt = context->GetModule<Network::ClientNetworkModule>();

        if (!networkModuleOpt.has_value() || !networkModuleOpt->get().IsConnected())
            return;

        Common::Network::Packets::PlayerPositionPacket posPacket;
        posPacket.playerId = pid;
        posPacket.x = x;
        posPacket.y = y;
        posPacket.z = z;
        posPacket.yaw = yaw;
        posPacket.pitch = pitch;

        networkModuleOpt->get().Send(posPacket);
    }

    std::vector<std::type_index> ClientPlayerModule::GetDependencies() const
    {
        return DependsOn<Network::ClientNetworkModule>();
    }
}
