#pragma once

#include "RenderStar/Common/Event/IEvent.hpp"
#include <string>

namespace RenderStar::Server::Event::Events
{
    struct ClientLeftEvent : public Common::Event::TypedEvent<ClientLeftEvent>
    {
        std::string remoteAddress;
        std::string reason;

        ClientLeftEvent(std::string address, std::string disconnectReason);

        std::string_view GetName() const override { return "ClientLeftEvent"; }
    };
}
