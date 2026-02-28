#pragma once

#include "RenderStar/Common/Network/IPacket.hpp"
#include "RenderStar/Common/Network/PacketBuffer.hpp"
#include <cstdint>

namespace RenderStar::Common::Network::Packets
{
    class EntityDestroyPacket final : public IPacket
    {
    public:

        int32_t serverEntityId = -1;

        void Write(PacketBuffer& buffer) const override
        {
            buffer.WriteInt32(serverEntityId);
        }

        void Read(PacketBuffer& buffer) override
        {
            serverEntityId = buffer.ReadInt32();
        }
    };
}
