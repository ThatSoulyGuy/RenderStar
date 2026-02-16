#pragma once

#include "RenderStar/Common/Event/IEvent.hpp"
#include "RenderStar/Server/Network/ClientConnection.hpp"
#include <memory>
#include <string>

namespace RenderStar::Server::Event::Events
{
    struct ClientJoinedEvent : public Common::Event::TypedEvent<ClientJoinedEvent>
    {
        std::string remoteAddress;
        std::shared_ptr<Network::ClientConnection> connection;

        ClientJoinedEvent(std::string address, std::shared_ptr<Network::ClientConnection> conn);

        std::string_view GetName() const override { return "ClientJoinedEvent"; }
    };
}
