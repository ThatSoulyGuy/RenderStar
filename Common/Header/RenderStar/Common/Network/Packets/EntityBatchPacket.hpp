#pragma once

#include "RenderStar/Common/Network/IPacket.hpp"
#include "RenderStar/Common/Network/PacketBuffer.hpp"
#include <cstdint>
#include <string>

namespace RenderStar::Common::Network::Packets
{
    class EntityBatchPacket final : public IPacket
    {
    public:

        int32_t batchIndex = 0;
        int32_t totalBatches = 0;
        std::string xmlData;

        void Write(PacketBuffer& buffer) const override
        {
            buffer.WriteInt32(batchIndex);
            buffer.WriteInt32(totalBatches);
            buffer.WriteString(xmlData);
        }

        void Read(PacketBuffer& buffer) override
        {
            batchIndex = buffer.ReadInt32();
            totalBatches = buffer.ReadInt32();
            xmlData = buffer.ReadString();
        }
    };
}
