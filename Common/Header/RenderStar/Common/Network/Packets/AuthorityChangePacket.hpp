#pragma once

#include "RenderStar/Common/Network/IPacket.hpp"
#include "RenderStar/Common/Network/PacketBuffer.hpp"

namespace RenderStar::Common::Network::Packets
{
    class AuthorityChangePacket final : public IPacket
    {
    public:

        int32_t entityId = -1;
        uint8_t authorityLevel = 0;
        int32_t ownerId = -1;

        void Write(PacketBuffer& buffer) const override
        {
            buffer.WriteInt32(entityId);
            buffer.WriteByte(static_cast<std::byte>(authorityLevel));
            buffer.WriteInt32(ownerId);
        }

        void Read(PacketBuffer& buffer) override
        {
            entityId = buffer.ReadInt32();
            authorityLevel = static_cast<uint8_t>(buffer.ReadByte());
            ownerId = buffer.ReadInt32();
        }
    };
}
