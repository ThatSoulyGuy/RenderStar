#include "RenderStar/Server/Event/Buses/ServerCoreEventBus.hpp"

namespace RenderStar::Server::Event::Buses
{
    ServerCoreEventBus::ServerCoreEventBus()
        : AbstractEventBus(true)
    {
    }

    std::string_view ServerCoreEventBus::GetBusName() const
    {
        return "server_event_bus_core";
    }
}
