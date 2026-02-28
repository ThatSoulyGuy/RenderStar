#pragma once

#include "RenderStar/Common/Network/IPacket.hpp"
#include "RenderStar/Common/Network/PacketBuffer.hpp"
#include <string>

namespace RenderStar::Common::Network::Packets
{
    class ComponentUpdatePacket final : public IPacket
    {
    public:

        int32_t entityId = -1;
        std::string xmlData;

        void Write(PacketBuffer& buffer) const override
        {
            buffer.WriteInt32(entityId);
            buffer.WriteString(xmlData);
        }

        void Read(PacketBuffer& buffer) override
        {
            entityId = buffer.ReadInt32();
            xmlData = buffer.ReadString();
        }
    };
}
