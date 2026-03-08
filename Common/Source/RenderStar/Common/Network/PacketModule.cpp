#include "RenderStar/Common/Network/PacketModule.hpp"
#include "RenderStar/Common/Network/Packets/EntityBatchPacket.hpp"
#include "RenderStar/Common/Network/Packets/EntityCreatePacket.hpp"
#include "RenderStar/Common/Network/Packets/EntityDestroyPacket.hpp"
#include "RenderStar/Common/Network/Packets/PlayerAssignPacket.hpp"
#include "RenderStar/Common/Network/Packets/SceneDataPacket.hpp"
#include "RenderStar/Common/Network/Packets/ComponentUpdatePacket.hpp"
#include "RenderStar/Common/Network/Packets/AuthorityChangePacket.hpp"
#include "RenderStar/Common/Network/Packets/PlayerInputPacket.hpp"
#include "RenderStar/Common/Network/Packets/PlayerStatePacket.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"

namespace RenderStar::Common::Network
{
    std::unique_ptr<IPacket> PacketModule::CreatePacket(PacketIdentifier packetId)
    {
        auto iterator = factories.find(packetId);

        if (iterator == factories.end())
        {
            logger->error("Unknown packet ID: {}", packetId);
            return nullptr;
        }

        return iterator->second();
    }

    PacketBuffer PacketModule::Serialize(const IPacket& packet)
    {
        auto typeIndex = std::type_index(typeid(packet));
        auto iterator = typeToId.find(typeIndex);

        if (iterator == typeToId.end())
        {
            logger->error("Unregistered packet type");
            return PacketBuffer::Allocate(0);
        }

        PacketBuffer buffer = PacketBuffer::Allocate();
        buffer.WriteVarint(iterator->second);
        packet.Write(buffer);
        return buffer;
    }

    std::unique_ptr<IPacket> PacketModule::Deserialize(PacketBuffer& buffer)
    {
        PacketIdentifier packetId = buffer.ReadVarint();
        auto packet = CreatePacket(packetId);

        if (packet != nullptr)
            packet->Read(buffer);

        return packet;
    }

    void PacketModule::HandlePacket(IPacket& packet)
    {
        auto typeIndex = std::type_index(typeid(packet));
        auto iterator = handlers.find(typeIndex);

        if (iterator != handlers.end())
            iterator->second(packet);
        else
            logger->warn("No handler registered for packet type '{}'", typeIndex.name());
    }

    void PacketModule::RegisterAllPackets()
    {
        RegisterPacket<Packets::PlayerAssignPacket>();
        RegisterPacket<Packets::SceneDataPacket>();
        RegisterPacket<Packets::EntityBatchPacket>();
        RegisterPacket<Packets::EntityCreatePacket>();
        RegisterPacket<Packets::EntityDestroyPacket>();
        RegisterPacket<Packets::ComponentUpdatePacket>();
        RegisterPacket<Packets::AuthorityChangePacket>();
        RegisterPacket<Packets::PlayerInputPacket>();
        RegisterPacket<Packets::PlayerStatePacket>();
    }

    void PacketModule::OnInitialize(Module::ModuleContext& context)
    {
        RegisterAllPackets();
        logger->info("PacketModule initialized with {} packet types", factories.size());
    }
}
