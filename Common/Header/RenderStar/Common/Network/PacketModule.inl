#pragma once

namespace RenderStar::Common::Network
{
    template<typename PacketType>
    void PacketModule::RegisterPacket(PacketIdentifier packetId)
    {
        auto typeIndex = std::type_index(typeid(PacketType));

        factories[packetId] = []() { return std::make_unique<PacketType>(); };
        idToType[packetId] = typeIndex;
        typeToId[typeIndex] = packetId;
    }

    template<typename PacketType>
    void PacketModule::RegisterHandler(std::function<void(PacketType&)> handler)
    {
        auto typeIndex = std::type_index(typeid(PacketType));

        handlers[typeIndex] = [handler](IPacket& packet)
        {
            handler(static_cast<PacketType&>(packet));
        };
    }

    template<typename PacketType>
    std::unique_ptr<PacketType> PacketModule::CreatePacket()
    {
        return std::make_unique<PacketType>();
    }
}
