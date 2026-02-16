#include "RenderStar/Server/Event/Events/ClientLeftEvent.hpp"

namespace RenderStar::Server::Event::Events
{
    ClientLeftEvent::ClientLeftEvent(std::string address, std::string disconnectReason, std::shared_ptr<Network::ClientConnection> conn)
        : remoteAddress(std::move(address))
        , reason(std::move(disconnectReason))
        , connection(std::move(conn))
    {
    }
}
