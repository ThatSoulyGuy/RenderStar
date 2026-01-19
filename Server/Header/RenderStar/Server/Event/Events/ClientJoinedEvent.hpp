#pragma once

#include "RenderStar/Common/Event/IEvent.hpp"
#include <string>

namespace RenderStar::Server::Event::Events
{
    struct ClientJoinedEvent : public Common::Event::Event<ClientJoinedEvent>
    {
        std::string remoteAddress;

        explicit ClientJoinedEvent(std::string address);

        std::string_view GetName() const override { return "ClientJoinedEvent"; }
    };
}
