#pragma once

#include <cstdint>

namespace RenderStar::Client::Input
{
    enum class KeyState : uint8_t
    {
        PRESSED,
        HELD,
        RELEASED
    };
}
