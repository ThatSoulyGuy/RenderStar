#pragma once

#include "RenderStar/Common/Event/IEvent.hpp"
#include "RenderStar/Server/Network/ClientConnection.hpp"
#include <memory>
#include <string>

namespace RenderStar::Server::Event::Events
{
    struct ClientLeftEvent : public Common::Event::TypedEvent<ClientLeftEvent>
    {
        std::string remoteAddress;
        std::string reason;
        std::shared_ptr<Network::ClientConnection> connection;

        ClientLeftEvent(std::string address, std::string disconnectReason, std::shared_ptr<Network::ClientConnection> conn);

        std::string_view GetName() const override { return "ClientLeftEvent"; }
    };
}
