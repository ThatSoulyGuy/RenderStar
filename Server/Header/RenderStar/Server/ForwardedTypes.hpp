#pragma once

namespace RenderStar::Server::Core
{
    class ServerLifecycleModule;
}

namespace RenderStar::Server::Event::Buses
{
    class ServerCoreEventBus;
}

namespace RenderStar::Server::Event::Events
{
    struct ServerPreinitializationEvent;
    struct ServerInitializationEvent;
    struct ClientJoinedEvent;
    struct ClientLeftEvent;
}

namespace RenderStar::Server::Network
{
    class ServerNetworkModule;
    struct ClientConnection;
}
