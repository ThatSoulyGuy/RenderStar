#include "RenderStar/Server/Event/Events/ClientJoinedEvent.hpp"

namespace RenderStar::Server::Event::Events
{
    ClientJoinedEvent::ClientJoinedEvent(std::string address, std::shared_ptr<Network::ClientConnection> conn)
        : remoteAddress(std::move(address))
        , connection(std::move(conn))
    {
    }
}
