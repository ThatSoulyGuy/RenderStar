#include "RenderStar/Client/Network/ClientNetworkModule.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"
#include "RenderStar/Common/Event/AbstractEventBus.hpp"
#include "RenderStar/Common/Network/PacketModule.hpp"
#include "RenderStar/Common/Network/IPacket.hpp"
#include "RenderStar/Common/Configuration/ConfigurationFactory.hpp"
#include "RenderStar/Client/Event/Buses/ClientCoreEventBus.hpp"
#include <algorithm>
#include <spdlog/spdlog.h>

namespace RenderStar::Client::Network
{
    ClientNetworkModule::ClientNetworkModule() : state(ConnectionState::DISCONNECTED), serverPort(0), connectionTimeoutMs(DEFAULT_CONNECTION_TIMEOUT_MS), localServerMaxPlayers(DEFAULT_LOCAL_SERVER_MAX_PLAYERS), coreEventBus(nullptr), receiveBuffer() { }

    ClientNetworkModule::~ClientNetworkModule()
    {
        Disconnect();
    }

    void ClientNetworkModule::OnInitialize(Common::Module::ModuleContext& context)
    {
        LoadConfiguration();

        RegisterSubModule<Common::Network::PacketModule>(std::make_unique<Common::Network::PacketModule>());

        const auto eventBus = context.GetEventBus<Event::ClientCoreEventBus>();

        if (eventBus.has_value())
            coreEventBus = &eventBus->get();

        Connect("127.0.0.1", 25565);
    }

    void ClientNetworkModule::Connect(const std::string& hostname, int32_t port)
    {
        if (auto expected = ConnectionState::DISCONNECTED; !state.compare_exchange_strong(expected, ConnectionState::CONNECTING))
        {
            logger->warn("Cannot connect - already connected or connecting");
            return;
        }

        serverAddress = hostname;
        serverPort = port;

        socket = std::make_unique<TcpSocket>(ioContext);
        TcpResolver resolver(ioContext);

        try
        {
            const auto endpoints = resolver.resolve(hostname, std::to_string(port));

            asio::async_connect(*socket, endpoints,
                [this](const asio::error_code& error, const asio::ip::tcp::endpoint& endpoint)
                {
                    if (error)
                    {
                        logger->error("Failed to connect to {}:{} - {}", serverAddress, serverPort, error.message());

                        state.store(ConnectionState::DISCONNECTED);
                        socket.reset();

                        return;
                    }

                    state.store(ConnectionState::CONNECTED);

                    logger->info("Connected to {}:{}", endpoint.address().to_string(), endpoint.port());

                    StartReading();
                });

            networkThread = std::thread(&ClientNetworkModule::NetworkThreadLoop, this);
        }
        catch (const std::exception& exception)
        {
            logger->error("Failed to resolve {}:{} - {}", hostname, port, exception.what());

            state.store(ConnectionState::DISCONNECTED);
            socket.reset();
        }
    }

    void ClientNetworkModule::Disconnect()
    {
        if (const ConnectionState currentState = state.load(); currentState == ConnectionState::DISCONNECTED)
            return;

        state.store(ConnectionState::DISCONNECTING);
        logger->info("Disconnecting from server...");

        ioContext.stop();

        if (socket && socket->is_open())
        {
            asio::error_code errorCode;

            socket->shutdown(asio::ip::tcp::socket::shutdown_both, errorCode);
            socket->close(errorCode);
        }

        if (networkThread.joinable())
            networkThread.join();

        socket.reset();
        state.store(ConnectionState::DISCONNECTED);

        logger->info("Disconnected from server");
    }

    void ClientNetworkModule::StartReading()
    {
        if (!socket || !socket->is_open())
            return;

        socket->async_read_some(asio::buffer(receiveBuffer.data(), receiveBuffer.size()), [this](const asio::error_code& error, size_t bytesReceived)
            {
                if (error)
                {
                    if (error == asio::error::eof)
                        HandleDisconnect("Server closed connection");
                    else if (error != asio::error::operation_aborted)
                        HandleDisconnect(error.message());

                    return;
                }

                ProcessReceivedData(bytesReceived);
                StartReading();
            });
    }

    void ClientNetworkModule::ProcessReceivedData(size_t bytesReceived)
    {
        auto packetModule = GetPacketModule();
        if (!packetModule)
            return;

        const std::span dataSpan(reinterpret_cast<const std::byte*>(receiveBuffer.data()), bytesReceived);
        Common::Network::PacketBuffer buffer = Common::Network::PacketBuffer::Wrap(dataSpan);

        const auto packet = packetModule->Deserialize(buffer);

        if (!packet)
        {
            logger->warn("Failed to deserialize packet from server");
            return;
        }

        packetModule->HandlePacket(*packet);
    }

    void ClientNetworkModule::HandleDisconnect(const std::string& reason)
    {
        if (const ConnectionState currentState = state.load(); currentState == ConnectionState::DISCONNECTED || currentState == ConnectionState::DISCONNECTING)
            return;

        logger->info("Connection lost: {}", reason);
        state.store(ConnectionState::DISCONNECTED);
    }

    void ClientNetworkModule::NetworkThreadLoop()
    {
        auto workGuard = asio::make_work_guard(ioContext);
        ioContext.run();
    }

    bool ClientNetworkModule::Send(const Common::Network::IPacket& packet)
    {
        if (state.load() != ConnectionState::CONNECTED || !socket || !socket->is_open())
        {
            logger->warn("Cannot send packet - not connected");
            return false;
        }

        auto packetModule = GetPacketModule();

        if (!packetModule)
            return false;

        const Common::Network::PacketBuffer serialized = packetModule->Serialize(packet);

        auto data = serialized.ToSpan();

        const uint32_t length = static_cast<uint32_t>(data.size());

        std::vector<uint8_t> buffer(4 + data.size());

        buffer[0] = static_cast<uint8_t>((length >> 24) & 0xFF);
        buffer[1] = static_cast<uint8_t>((length >> 16) & 0xFF);
        buffer[2] = static_cast<uint8_t>((length >> 8) & 0xFF);
        buffer[3] = static_cast<uint8_t>(length & 0xFF);

        std::ranges::transform(data, buffer.begin() + 4, [](std::byte b) { return static_cast<uint8_t>(b); });

        asio::error_code errorCode;
        asio::write(*socket, asio::buffer(buffer), errorCode);

        if (errorCode)
        {
            logger->error("Failed to send packet: {}", errorCode.message());
            return false;
        }

        return true;
    }

    Common::Network::PacketModule* ClientNetworkModule::GetPacketModule() const
    {
        const auto subModule = const_cast<ClientNetworkModule*>(this)->GetSubModule<Common::Network::PacketModule>();

        if (subModule.has_value())
            return &subModule->get();

        return nullptr;
    }

    ConnectionState ClientNetworkModule::GetState() const
    {
        return state.load();
    }

    bool ClientNetworkModule::IsConnected() const
    {
        return state.load() == ConnectionState::CONNECTED;
    }

    bool ClientNetworkModule::IsConnecting() const
    {
        return state.load() == ConnectionState::CONNECTING;
    }

    std::string ClientNetworkModule::GetServerAddress() const
    {
        return serverAddress;
    }

    int32_t ClientNetworkModule::GetServerPort() const
    {
        return serverPort;
    }

    int32_t ClientNetworkModule::GetConnectionTimeoutMs() const
    {
        return connectionTimeoutMs;
    }

    int32_t ClientNetworkModule::GetLocalServerMaxPlayers() const
    {
        return localServerMaxPlayers;
    }

    void ClientNetworkModule::LoadConfiguration()
    {
        const auto config = GetConfiguration();

        if (const auto timeoutOpt = config->GetInteger("connection_timeout_ms"); timeoutOpt.has_value())
            connectionTimeoutMs = timeoutOpt.value();

        if (const auto maxPlayersOpt = config->GetInteger("local_server_max_players"); maxPlayersOpt.has_value())
            localServerMaxPlayers = maxPlayersOpt.value();

        logger->debug("Loaded configuration: connection_timeout={}ms, local_max_players={}", connectionTimeoutMs, localServerMaxPlayers);
    }

    std::shared_ptr<Common::Configuration::IConfiguration> ClientNetworkModule::GetConfiguration()
    {
        return Common::Configuration::ConfigurationFactory::For<ClientNetworkModule>("render_star");
    }
}
