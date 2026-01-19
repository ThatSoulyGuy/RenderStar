#pragma once

#include "RenderStar/Common/Event/IEvent.hpp"
#include <string_view>

namespace RenderStar::Common::Event::Events
{
    struct CommonSignalTerminateEvent final : Event<CommonSignalTerminateEvent>
    {
        std::string_view GetName() const override { return "CommonSignalTerminateEvent"; }
    };
}
