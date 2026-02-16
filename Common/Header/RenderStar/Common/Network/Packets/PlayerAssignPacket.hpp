#pragma once

#include "RenderStar/Common/Network/IPacket.hpp"
#include "RenderStar/Common/Network/PacketBuffer.hpp"
#include <cstdint>

namespace RenderStar::Common::Network::Packets
{
    class PlayerAssignPacket final : public IPacket
    {
    public:

        int32_t playerId = -1;

        void Write(PacketBuffer& buffer) const override
        {
            buffer.WriteInt32(playerId);
        }

        void Read(PacketBuffer& buffer) override
        {
            playerId = buffer.ReadInt32();
        }
    };
}
