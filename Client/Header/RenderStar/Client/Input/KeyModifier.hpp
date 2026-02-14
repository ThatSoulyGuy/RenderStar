#pragma once

#include <cstdint>

namespace RenderStar::Client::Input
{
    enum class KeyModifier : uint8_t
    {
        NONE    = 0x00,
        SHIFT   = 0x01,
        CTRL    = 0x02,
        ALT     = 0x04,
        SUPER   = 0x08
    };

    inline KeyModifier operator|(KeyModifier lhs, KeyModifier rhs)
    {
        return static_cast<KeyModifier>(
            static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs)
        );
    }

    inline KeyModifier operator&(KeyModifier lhs, KeyModifier rhs)
    {
        return static_cast<KeyModifier>(
            static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs)
        );
    }

    inline bool HasModifier(KeyModifier flags, KeyModifier test)
    {
        return (flags & test) == test;
    }
}
