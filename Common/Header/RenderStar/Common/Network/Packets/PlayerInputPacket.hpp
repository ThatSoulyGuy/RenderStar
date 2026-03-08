#pragma once

#include "RenderStar/Common/Network/IPacket.hpp"
#include "RenderStar/Common/Network/PacketBuffer.hpp"
#include <cstdint>

namespace RenderStar::Common::Network::Packets
{
    class PlayerInputPacket final : public IPacket
    {
    public:

        static constexpr uint8_t FLAG_FORWARD  = 1 << 0;
        static constexpr uint8_t FLAG_BACKWARD = 1 << 1;
        static constexpr uint8_t FLAG_LEFT     = 1 << 2;
        static constexpr uint8_t FLAG_RIGHT    = 1 << 3;
        static constexpr uint8_t FLAG_JUMP     = 1 << 4;

        int32_t sequenceNumber = 0;
        uint8_t inputFlags = 0;
        float yaw = 0.0f;
        float pitch = 0.0f;
        float deltaTime = 0.0f;

        void Write(PacketBuffer& buffer) const override
        {
            buffer.WriteInt32(sequenceNumber);
            buffer.WriteByte(static_cast<std::byte>(inputFlags));
            buffer.WriteFloat(yaw);
            buffer.WriteFloat(pitch);
            buffer.WriteFloat(deltaTime);
        }

        void Read(PacketBuffer& buffer) override
        {
            sequenceNumber = buffer.ReadInt32();
            inputFlags = static_cast<uint8_t>(buffer.ReadByte());
            yaw = buffer.ReadFloat();
            pitch = buffer.ReadFloat();
            deltaTime = buffer.ReadFloat();
        }
    };
}
