#pragma once

#include "RenderStar/Common/Network/IPacket.hpp"
#include "RenderStar/Common/Network/PacketBuffer.hpp"
#include <cstdint>

namespace RenderStar::Common::Network::Packets
{
    class PlayerPositionPacket final : public IPacket
    {
    public:

        int32_t playerId = -1;
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float yaw = -90.0f;
        float pitch = 0.0f;

        void Write(PacketBuffer& buffer) const override
        {
            buffer.WriteInt32(playerId);
            buffer.WriteFloat(x);
            buffer.WriteFloat(y);
            buffer.WriteFloat(z);
            buffer.WriteFloat(yaw);
            buffer.WriteFloat(pitch);
        }

        void Read(PacketBuffer& buffer) override
        {
            playerId = buffer.ReadInt32();
            x = buffer.ReadFloat();
            y = buffer.ReadFloat();
            z = buffer.ReadFloat();
            yaw = buffer.ReadFloat();
            pitch = buffer.ReadFloat();
        }
    };
}
