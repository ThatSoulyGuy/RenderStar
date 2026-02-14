#pragma once

#include <cstdint>

namespace RenderStar::Client::Input
{
    enum class MouseState : uint8_t
    {
        PRESSED,
        HELD,
        RELEASED
    };
}
