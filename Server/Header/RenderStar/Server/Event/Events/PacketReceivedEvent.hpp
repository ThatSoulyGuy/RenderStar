#pragma once

#include "RenderStar/Common/Event/IEvent.hpp"
#include "RenderStar/Server/Network/ClientConnection.hpp"
#include <memory>

namespace RenderStar::Common::Network
{
    class IPacket;
}

namespace RenderStar::Server::Event::Events
{
    struct PacketReceivedEvent : public Common::Event::TypedEvent<PacketReceivedEvent>
    {
        std::shared_ptr<Network::ClientConnection> connection;
        Common::Network::IPacket* packet;

        PacketReceivedEvent(std::shared_ptr<Network::ClientConnection> conn, Common::Network::IPacket* pkt);

        std::string_view GetName() const override { return "PacketReceivedEvent"; }
    };
}
