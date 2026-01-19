#pragma once

#include "RenderStar/Common/Module/AbstractModule.hpp"
#include "RenderStar/Common/Configuration/IConfiguration.hpp"
#include "RenderStar/Server/Network/ServerMode.hpp"
#include "RenderStar/Server/Network/ClientConnection.hpp"
#include <asio.hpp>
#include <memory>
#include <atomic>
#include <thread>
#include <unordered_map>

namespace RenderStar::Common::Event
{
    class AbstractEventBus;
}

namespace RenderStar::Common::Network
{
    class IPacket;
    class PacketModule;
}

namespace RenderStar::Server::Network
{
    class ServerNetworkModule : public Common::Module::AbstractModule
    {
    public:

        static constexpr int32_t DEFAULT_PORT = 25565;
        static constexpr int32_t DEFAULT_MAX_PLAYERS = 20;

        ServerNetworkModule(ServerMode serverMode, int32_t serverPort, int32_t maximumPlayers);

        ~ServerNetworkModule() override;

        static std::unique_ptr<ServerNetworkModule> Dedicated();

        static std::unique_ptr<ServerNetworkModule> Local(int32_t port, int32_t maxPlayers);

        static std::unique_ptr<ServerNetworkModule> FromArguments(int argc, char** argv);

        void StartServer();

        void StopServer();

        bool Send(ClientConnection& connection, const Common::Network::IPacket& packet);

        void Broadcast(const Common::Network::IPacket& packet);

        void Broadcast(const Common::Network::IPacket& packet, const ClientConnection& exclude);

        void Disconnect(ClientConnection& connection, const std::string& reason);

        Common::Network::PacketModule* GetPacketModule() const;

        ServerMode GetMode() const;

        int32_t GetPort() const;

        int32_t GetMaxPlayers() const;

        int32_t GetPlayerCount() const;

        bool IsRunning() const;

        bool IsFull() const;

    protected:

        void OnInitialize(Common::Module::ModuleContext& context) override;

    private:

        using TcpAcceptor = asio::ip::tcp::acceptor;
        using TcpSocket = asio::ip::tcp::socket;
        using SocketPointer = std::shared_ptr<TcpSocket>;
        using ConnectionPointer = std::shared_ptr<ClientConnection>;

        void AcceptConnections();

        void ReadFromClient(ConnectionPointer connection);

        void ProcessPackets(ConnectionPointer connection, size_t bytesReceived);

        void RemoveConnection(ConnectionPointer connection);

        static std::shared_ptr<Common::Configuration::IConfiguration> GetConfiguration();

        ServerMode mode;
        int32_t port;
        int32_t maxPlayers;
        std::atomic<bool> running;
        Common::Event::AbstractEventBus* coreEventBus;

        asio::io_context ioContext;
        std::unique_ptr<TcpAcceptor> acceptor;
        std::thread networkThread;
        std::unordered_map<TcpSocket*, ConnectionPointer> connections;
        std::mutex connectionsMutex;
    };
}
