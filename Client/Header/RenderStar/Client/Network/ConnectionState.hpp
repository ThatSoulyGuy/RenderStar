#pragma once

#include <cstdint>

namespace RenderStar::Client::Network
{
    enum class ConnectionState : uint8_t
    {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        DISCONNECTING
    };
}
