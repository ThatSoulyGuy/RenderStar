#pragma once

#include "RenderStar/Common/Module/AbstractModule.hpp"
#include "RenderStar/Common/Network/IPacket.hpp"
#include "RenderStar/Common/Network/PacketBuffer.hpp"
#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>

namespace RenderStar::Common::Network
{
    using PacketIdentifier = int32_t;
    using PacketFactory = std::function<std::unique_ptr<IPacket>()>;
    using PacketHandler = std::function<void(IPacket&)>;

    class PacketModule : public Module::AbstractModule
    {
    public:

        template<typename PacketType>
        void RegisterPacket(PacketIdentifier packetId);

        template<typename PacketType>
        void RegisterHandler(std::function<void(PacketType&)> handler);

        std::unique_ptr<IPacket> CreatePacket(PacketIdentifier packetId);

        template<typename PacketType>
        std::unique_ptr<PacketType> CreatePacket();

        PacketBuffer Serialize(const IPacket& packet);

        std::unique_ptr<IPacket> Deserialize(PacketBuffer& buffer);

        void HandlePacket(IPacket& packet);

    protected:

        void OnInitialize(Module::ModuleContext& context) override;

    private:

        std::unordered_map<PacketIdentifier, PacketFactory> factories;
        std::unordered_map<PacketIdentifier, std::type_index> idToType;
        std::unordered_map<std::type_index, PacketIdentifier> typeToId;
        std::unordered_map<std::type_index, PacketHandler> handlers;
    };
}

#include "RenderStar/Common/Network/PacketModule.inl"
