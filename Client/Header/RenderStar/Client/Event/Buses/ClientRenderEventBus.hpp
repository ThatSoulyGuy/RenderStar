#pragma once

#include "RenderStar/Common/Event/AbstractEventBus.hpp"

namespace RenderStar::Client::Event
{
    class ClientRenderEventBus final : public Common::Event::AbstractEventBus
    {
    public:

        ClientRenderEventBus();

    protected:

        std::string_view GetBusName() const override;
    };
}
