#pragma once

namespace RenderStar::Common::Network
{
    template <typename PacketType>
    void PacketModule::RegisterPacket()
    {
        const auto typeIndex = std::type_index(typeid(PacketType));

        if (typeToId.contains(typeIndex))
            return;

        PacketIdentifier packetId = nextId++;

        factories[packetId] = []() { return std::make_unique<PacketType>(); };
        idToType.insert_or_assign(packetId, typeIndex);
        typeToId.insert_or_assign(typeIndex, packetId);
    }

    template <typename PacketType>
    void PacketModule::RegisterHandler(std::function<void(PacketType&)> handler)
    {
        const auto typeIndex = std::type_index(typeid(PacketType));

        handlers[typeIndex] = [handler](IPacket& packet)
        {
            handler(static_cast<PacketType&>(packet));
        };
    }

    template <typename PacketType>
    std::unique_ptr<PacketType> PacketModule::CreatePacket()
    {
        return std::make_unique<PacketType>();
    }
}
