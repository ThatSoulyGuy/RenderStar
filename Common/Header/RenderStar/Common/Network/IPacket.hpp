#pragma once

namespace RenderStar::Common::Network
{
    class PacketBuffer;

    class IPacket
    {
    public:

        virtual ~IPacket() = default;

        virtual void Write(PacketBuffer& buffer) const = 0;

        virtual void Read(PacketBuffer& buffer) = 0;
    };
}
