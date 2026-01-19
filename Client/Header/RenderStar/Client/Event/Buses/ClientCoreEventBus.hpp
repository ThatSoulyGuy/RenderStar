#pragma once

#include "RenderStar/Common/Event/AbstractEventBus.hpp"

namespace RenderStar::Client::Event
{
    class ClientCoreEventBus final : public Common::Event::AbstractEventBus
    {
    public:

        ClientCoreEventBus();

    protected:

        std::string_view GetBusName() const override;
    };
}
