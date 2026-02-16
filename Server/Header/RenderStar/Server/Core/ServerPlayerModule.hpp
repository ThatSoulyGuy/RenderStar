#pragma once

#include "RenderStar/Common/Module/AbstractModule.hpp"
#include "RenderStar/Server/Network/ClientConnection.hpp"
#include <cstdint>
#include <memory>
#include <unordered_map>

namespace RenderStar::Common::Network
{
    class PacketModule;
    class IPacket;
}

namespace RenderStar::Server::Network
{
    class ServerNetworkModule;
}

namespace RenderStar::Server::Core
{
    struct PlayerState
    {
        std::shared_ptr<Network::ClientConnection> connection;
        float posX = 0.0f;
        float posY = 2.0f;
        float posZ = 5.0f;
        float yaw = -90.0f;
        float pitch = 0.0f;
    };

    class ServerPlayerModule final : public Common::Module::AbstractModule
    {
    public:

        [[nodiscard]]
        std::vector<std::type_index> GetDependencies() const override;

    protected:

        void OnInitialize(Common::Module::ModuleContext& context) override;

    private:

        void OnClientJoined(std::shared_ptr<Network::ClientConnection> connection);
        void OnClientLeft(std::shared_ptr<Network::ClientConnection> connection);
        void OnPacketReceived(std::shared_ptr<Network::ClientConnection> connection, Common::Network::IPacket& packet);

        Network::ServerNetworkModule* networkModule = nullptr;
        Common::Network::PacketModule* packetModule = nullptr;

        int32_t nextPlayerId = 0;
        std::unordered_map<int32_t, PlayerState> players;
    };
}
