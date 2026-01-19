#pragma once

#include "RenderStar/Common/Event/AbstractEventBus.hpp"

namespace RenderStar::Server::Event::Buses
{
    class ServerCoreEventBus : public Common::Event::AbstractEventBus
    {
    public:

        ServerCoreEventBus();

    protected:

        std::string_view GetBusName() const override;
    };
}
