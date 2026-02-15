#include "RenderStar/Client/Event/Buses/ClientCoreEventBus.hpp"
#include "RenderStar/Client/Event/Buses/ClientRenderEventBus.hpp"

namespace RenderStar::Client::Event
{
    ClientCoreEventBus::ClientCoreEventBus() : AbstractEventBus(true) { }

    std::string_view ClientCoreEventBus::GetBusName() const
    {
        return "ClientCoreEventBus";
    }

    ClientRenderEventBus::ClientRenderEventBus() : AbstractEventBus(true) { }

    std::string_view ClientRenderEventBus::GetBusName() const
    {
        return "ClientRenderEventBus";
    }
}
