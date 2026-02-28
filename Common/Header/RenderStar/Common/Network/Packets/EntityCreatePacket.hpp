#pragma once

#include "RenderStar/Common/Network/IPacket.hpp"
#include "RenderStar/Common/Network/PacketBuffer.hpp"
#include <string>

namespace RenderStar::Common::Network::Packets
{
    class EntityCreatePacket final : public IPacket
    {
    public:

        std::string xmlData;

        void Write(PacketBuffer& buffer) const override
        {
            buffer.WriteString(xmlData);
        }

        void Read(PacketBuffer& buffer) override
        {
            xmlData = buffer.ReadString();
        }
    };
}
