#include "RenderStar/Server/Event/Events/ClientJoinedEvent.hpp"

namespace RenderStar::Server::Event::Events
{
    ClientJoinedEvent::ClientJoinedEvent(std::string address)
        : remoteAddress(std::move(address))
    {
    }
}
