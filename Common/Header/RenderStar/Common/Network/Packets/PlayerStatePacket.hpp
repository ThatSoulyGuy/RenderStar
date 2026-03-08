#pragma once

#include "RenderStar/Common/Network/IPacket.hpp"
#include "RenderStar/Common/Network/PacketBuffer.hpp"
#include <cstdint>

namespace RenderStar::Common::Network::Packets
{
    class PlayerStatePacket final : public IPacket
    {
    public:

        int32_t playerId = -1;
        int32_t lastProcessedSequence = 0;
        float posX = 0.0f;
        float posY = 0.0f;
        float posZ = 0.0f;
        float yaw = 0.0f;
        float pitch = 0.0f;
        bool grounded = false;
        double serverTime = 0.0;

        void Write(PacketBuffer& buffer) const override
        {
            buffer.WriteInt32(playerId);
            buffer.WriteInt32(lastProcessedSequence);
            buffer.WriteFloat(posX);
            buffer.WriteFloat(posY);
            buffer.WriteFloat(posZ);
            buffer.WriteFloat(yaw);
            buffer.WriteFloat(pitch);
            buffer.WriteBoolean(grounded);
            buffer.WriteDouble(serverTime);
        }

        void Read(PacketBuffer& buffer) override
        {
            playerId = buffer.ReadInt32();
            lastProcessedSequence = buffer.ReadInt32();
            posX = buffer.ReadFloat();
            posY = buffer.ReadFloat();
            posZ = buffer.ReadFloat();
            yaw = buffer.ReadFloat();
            pitch = buffer.ReadFloat();
            grounded = buffer.ReadBoolean();
            serverTime = buffer.ReadDouble();
        }
    };
}
