#pragma once

#include "RenderStar/Common/Event/IEvent.hpp"

namespace RenderStar::Server::Event::Events
{
    struct ServerPreinitializationEvent : public Common::Event::TypedEvent<ServerPreinitializationEvent>
    {
        std::string_view GetName() const override { return "ServerPreinitializationEvent"; }
    };
}
