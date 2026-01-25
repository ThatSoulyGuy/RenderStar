#pragma once

#include "RenderStar/Common/Event/IEvent.hpp"

namespace RenderStar::Server::Event::Events
{
    struct ServerInitializationEvent : public Common::Event::TypedEvent<ServerInitializationEvent>
    {
        std::string_view GetName() const override { return "ServerInitializationEvent"; }
    };
}
