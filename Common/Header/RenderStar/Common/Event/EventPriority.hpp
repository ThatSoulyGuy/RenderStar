#pragma once

#include <cstdint>

namespace RenderStar::Common::Event
{
    enum class EventPriority : int32_t
    {
        HIGHEST = 0,
        HIGH = 1,
        NORMAL = 2,
        LOW = 3,
        LOWEST = 4
    };
}
