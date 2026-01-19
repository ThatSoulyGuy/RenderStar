#include "RenderStar/Server/Network/ServerNetworkModule.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"
#include "RenderStar/Common/Event/AbstractEventBus.hpp"
#include "RenderStar/Common/Event/EventResult.hpp"
#include "RenderStar/Common/Network/PacketModule.hpp"
#include "RenderStar/Common/Network/IPacket.hpp"
#include "RenderStar/Common/Configuration/ConfigurationFactory.hpp"
#include "RenderStar/Server/Event/Buses/ServerCoreEventBus.hpp"
#include "RenderStar/Server/Event/Events/ServerPreinitializationEvent.hpp"
#include "RenderStar/Server/Event/Events/ClientJoinedEvent.hpp"
#include "RenderStar/Server/Event/Events/ClientLeftEvent.hpp"
#include <algorithm>
#include <spdlog/spdlog.h>

namespace RenderStar::Server::Network
{
    ServerNetworkModule::ServerNetworkModule(ServerMode serverMode, int32_t serverPort, int32_t maximumPlayers)
        : mode(serverMode)
        , port(serverPort)
        , maxPlayers(maximumPlayers)
        , running(false)
        , coreEventBus(nullptr)
    {
    }

    ServerNetworkModule::~ServerNetworkModule()
    {
        StopServer();
    }

    std::unique_ptr<ServerNetworkModule> ServerNetworkModule::Dedicated()
    {
        auto config = GetConfiguration();

        int32_t serverPort = DEFAULT_PORT;
        int32_t maximumPlayers = DEFAULT_MAX_PLAYERS;

        auto portOpt = config->GetInteger("port");
        if (portOpt.has_value())
            serverPort = portOpt.value();

        auto maxPlayersOpt = config->GetInteger("max_players");
        if (maxPlayersOpt.has_value())
            maximumPlayers = maxPlayersOpt.value();

        spdlog::debug("Loaded server configuration: port={}, max_players={}", serverPort, maximumPlayers);

        return std::make_unique<ServerNetworkModule>(ServerMode::DEDICATED, serverPort, maximumPlayers);
    }

    std::unique_ptr<ServerNetworkModule> ServerNetworkModule::Local(int32_t localPort, int32_t localMaxPlayers)
    {
        return std::make_unique<ServerNetworkModule>(ServerMode::LOCAL, localPort, localMaxPlayers);
    }

    std::unique_ptr<ServerNetworkModule> ServerNetworkModule::FromArguments(int argc, char** argv)
    {
        ServerMode serverMode = ServerMode::DEDICATED;
        int32_t serverPort = -1;
        int32_t maximumPlayers = -1;

        for (int i = 1; i < argc; ++i)
        {
            std::string arg(argv[i]);

            if (arg.starts_with("--mode="))
            {
                std::string modeString = arg.substr(7);
                if (modeString == "LOCAL" || modeString == "local")
                    serverMode = ServerMode::LOCAL;
            }
            else if (arg.starts_with("--port="))
            {
                serverPort = std::stoi(arg.substr(7));
            }
            else if (arg.starts_with("--max-players="))
            {
                maximumPlayers = std::stoi(arg.substr(14));
            }
        }

        if (serverMode == ServerMode::LOCAL)
        {
            if (serverPort == -1)
                serverPort = DEFAULT_PORT;
            if (maximumPlayers == -1)
                maximumPlayers = 1;

            return Local(serverPort, maximumPlayers);
        }

        return Dedicated();
    }

    void ServerNetworkModule::OnInitialize(Common::Module::ModuleContext& context)
    {
        RegisterSubModule<Common::Network::PacketModule>(std::make_unique<Common::Network::PacketModule>());

        auto eventBus = context.GetEventBus<Event::Buses::ServerCoreEventBus>();
        if (eventBus.has_value())
        {
            coreEventBus = &eventBus->get();

            eventBus->get().Subscribe<Event::Events::ServerPreinitializationEvent>(
                [this](const Event::Events::ServerPreinitializationEvent&)
                {
                    StartServer();
                    return Common::Event::EventResult::Success();
                });
        }
    }

    void ServerNetworkModule::StartServer()
    {
        bool expected = false;
        if (!running.compare_exchange_strong(expected, true))
        {
            logger->warn("Server is already running");
            return;
        }

        acceptor = std::make_unique<TcpAcceptor>(ioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), static_cast<uint16_t>(port)));

        AcceptConnections();

        networkThread = std::thread([this]()
        {
            ioContext.run();
        });

        logger->info("Server started on port {} in {} mode (max players: {})",
            port, mode == ServerMode::DEDICATED ? "DEDICATED" : "LOCAL", maxPlayers);
    }

    void ServerNetworkModule::StopServer()
    {
        bool expected = true;
        if (!running.compare_exchange_strong(expected, false))
            return;

        logger->info("Stopping server...");

        ioContext.stop();

        {
            std::lock_guard<std::mutex> lock(connectionsMutex);
            for (auto& [socket, connection] : connections)
                connection->Close();
            connections.clear();
        }

        if (networkThread.joinable())
            networkThread.join();

        if (acceptor)
            acceptor->close();

        logger->info("Server stopped");
    }

    void ServerNetworkModule::AcceptConnections()
    {
        auto socket = std::make_shared<TcpSocket>(ioContext);

        acceptor->async_accept(*socket, [this, socket](const asio::error_code& error)
        {
            if (error)
            {
                if (running.load())
                    logger->error("Accept error: {}", error.message());
                return;
            }

            {
                std::lock_guard<std::mutex> lock(connectionsMutex);
                if (static_cast<int32_t>(connections.size()) >= maxPlayers)
                {
                    logger->info("Rejecting connection - server full ({}/{})", connections.size(), maxPlayers);
                    socket->close();
                    AcceptConnections();
                    return;
                }

                auto connection = std::make_shared<ClientConnection>(socket);
                connections[socket.get()] = connection;

                logger->info("Client connected from {} ({}/{})",
                    connection->remoteAddress, connections.size(), maxPlayers);

                if (coreEventBus)
                    coreEventBus->Publish(Event::Events::ClientJoinedEvent(connection->remoteAddress));

                ReadFromClient(connection);
            }

            AcceptConnections();
        });
    }

    void ServerNetworkModule::ReadFromClient(ConnectionPointer connection)
    {
        auto& buffer = connection->receiveBuffer;

        connection->socket->async_read_some(
            asio::buffer(buffer.data(), buffer.size()),
            [this, connection](const asio::error_code& error, size_t bytesReceived)
            {
                if (error)
                {
                    std::string reason = error == asio::error::eof ? "Client closed connection" : error.message();
                    RemoveConnection(connection);
                    return;
                }

                ProcessPackets(connection, bytesReceived);
                ReadFromClient(connection);
            });
    }

    void ServerNetworkModule::ProcessPackets(ConnectionPointer connection, size_t bytesReceived)
    {
        auto packetModule = GetPacketModule();
        if (!packetModule)
            return;

        std::span<const std::byte> dataSpan(
            reinterpret_cast<const std::byte*>(connection->receiveBuffer.data()),
            bytesReceived);
        Common::Network::PacketBuffer buffer = Common::Network::PacketBuffer::Wrap(dataSpan);

        auto packet = packetModule->Deserialize(buffer);
        if (!packet)
        {
            logger->warn("Failed to deserialize packet from {}", connection->remoteAddress);
            return;
        }

        packetModule->HandlePacket(*packet);
    }

    void ServerNetworkModule::RemoveConnection(ConnectionPointer connection)
    {
        std::lock_guard<std::mutex> lock(connectionsMutex);

        auto iterator = connections.find(connection->socket.get());
        if (iterator != connections.end())
        {
            std::string address = connection->remoteAddress;
            connection->Close();
            connections.erase(iterator);

            logger->info("Client disconnected from {} ({}/{})", address, connections.size(), maxPlayers);

            if (coreEventBus)
                coreEventBus->Publish(Event::Events::ClientLeftEvent(address, "Disconnected"));
        }
    }

    bool ServerNetworkModule::Send(ClientConnection& connection, const Common::Network::IPacket& packet)
    {
        auto packetModule = GetPacketModule();
        if (!packetModule)
            return false;

        Common::Network::PacketBuffer serialized = packetModule->Serialize(packet);
        auto data = serialized.ToSpan();

        uint32_t length = static_cast<uint32_t>(data.size());
        std::vector<uint8_t> buffer(4 + data.size());
        buffer[0] = static_cast<uint8_t>((length >> 24) & 0xFF);
        buffer[1] = static_cast<uint8_t>((length >> 16) & 0xFF);
        buffer[2] = static_cast<uint8_t>((length >> 8) & 0xFF);
        buffer[3] = static_cast<uint8_t>(length & 0xFF);
        std::transform(data.begin(), data.end(), buffer.begin() + 4,
            [](std::byte b) { return static_cast<uint8_t>(b); });

        asio::error_code errorCode;
        asio::write(*connection.socket, asio::buffer(buffer), errorCode);

        if (errorCode)
        {
            logger->error("Failed to send packet: {}", errorCode.message());
            return false;
        }

        return true;
    }

    void ServerNetworkModule::Broadcast(const Common::Network::IPacket& packet)
    {
        std::lock_guard<std::mutex> lock(connectionsMutex);
        for (auto& [socket, connection] : connections)
            Send(*connection, packet);
    }

    void ServerNetworkModule::Broadcast(const Common::Network::IPacket& packet, const ClientConnection& exclude)
    {
        std::lock_guard<std::mutex> lock(connectionsMutex);
        for (auto& [socket, connection] : connections)
        {
            if (connection->socket.get() != exclude.socket.get())
                Send(*connection, packet);
        }
    }

    void ServerNetworkModule::Disconnect(ClientConnection& connection, const std::string& reason)
    {
        std::lock_guard<std::mutex> lock(connectionsMutex);

        auto iterator = connections.find(connection.socket.get());
        if (iterator != connections.end())
        {
            std::string address = connection.remoteAddress;
            connection.Close();
            connections.erase(iterator);

            logger->info("Client disconnected from {}: {} ({}/{})", address, reason, connections.size(), maxPlayers);

            if (coreEventBus)
                coreEventBus->Publish(Event::Events::ClientLeftEvent(address, reason));
        }
    }

    Common::Network::PacketModule* ServerNetworkModule::GetPacketModule() const
    {
        auto subModule = const_cast<ServerNetworkModule*>(this)->GetSubModule<Common::Network::PacketModule>();
        if (subModule.has_value())
            return &subModule->get();
        return nullptr;
    }

    ServerMode ServerNetworkModule::GetMode() const
    {
        return mode;
    }

    int32_t ServerNetworkModule::GetPort() const
    {
        return port;
    }

    int32_t ServerNetworkModule::GetMaxPlayers() const
    {
        return maxPlayers;
    }

    int32_t ServerNetworkModule::GetPlayerCount() const
    {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(connectionsMutex));
        return static_cast<int32_t>(connections.size());
    }

    bool ServerNetworkModule::IsRunning() const
    {
        return running.load();
    }

    bool ServerNetworkModule::IsFull() const
    {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(connectionsMutex));
        return static_cast<int32_t>(connections.size()) >= maxPlayers;
    }

    std::shared_ptr<Common::Configuration::IConfiguration> ServerNetworkModule::GetConfiguration()
    {
        return Common::Configuration::ConfigurationFactory::For<ServerNetworkModule>("render_star", "server_settings.xml");
    }
}
