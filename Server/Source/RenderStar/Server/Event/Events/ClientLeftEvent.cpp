#include "RenderStar/Server/Event/Events/ClientLeftEvent.hpp"

namespace RenderStar::Server::Event::Events
{
    ClientLeftEvent::ClientLeftEvent(std::string address, std::string disconnectReason)
        : remoteAddress(std::move(address))
        , reason(std::move(disconnectReason))
    {
    }
}
