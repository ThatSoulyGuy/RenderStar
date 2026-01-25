#pragma once

#include "RenderStar/Common/Module/AbstractModule.hpp"
#include "RenderStar/Client/Network/ConnectionState.hpp"
#include <asio.hpp>
#include <atomic>
#include <thread>
#include <queue>
#include <mutex>

namespace RenderStar::Common::Event
{
    class AbstractEventBus;
}

namespace RenderStar::Common::Network
{
    class IPacket;
    class PacketModule;
}

namespace RenderStar::Client::Network
{
    class ClientNetworkModule final : public Common::Module::AbstractModule
    {
    public:

        static constexpr int32_t DEFAULT_PORT = 25565;
        static constexpr int32_t DEFAULT_CONNECTION_TIMEOUT_MS = 5000;
        static constexpr int32_t DEFAULT_LOCAL_SERVER_MAX_PLAYERS = 8;
        static constexpr size_t RECEIVE_BUFFER_SIZE = 65536;

        ClientNetworkModule();

        ~ClientNetworkModule() override;

        void Connect(const std::string& hostname, int32_t serverPort);

        void Disconnect();

        bool Send(const Common::Network::IPacket& packet);

        Common::Network::PacketModule* GetPacketModule() const;

        ConnectionState GetState() const;

        bool IsConnected() const;

        bool IsConnecting() const;

        std::string GetServerAddress() const;

        int32_t GetServerPort() const;

        int32_t GetConnectionTimeoutMs() const;

        int32_t GetLocalServerMaxPlayers() const;

    protected:

        void OnInitialize(Common::Module::ModuleContext& context) override;

    private:

        using TcpSocket = asio::ip::tcp::socket;
        using TcpResolver = asio::ip::tcp::resolver;

        void StartReading();

        void ProcessReceivedData(size_t bytesReceived);

        void HandleDisconnect(const std::string& reason);

        void NetworkThreadLoop();

        void LoadConfiguration(Common::Module::ModuleContext& context);

        std::atomic<ConnectionState> state;
        std::string serverAddress;
        int32_t serverPort;
        int32_t connectionTimeoutMs;
        int32_t localServerMaxPlayers;
        Common::Event::AbstractEventBus* coreEventBus;

        asio::io_context ioContext;
        std::unique_ptr<TcpSocket> socket;
        std::thread networkThread;
        std::array<uint8_t, RECEIVE_BUFFER_SIZE> receiveBuffer;
    };
}
