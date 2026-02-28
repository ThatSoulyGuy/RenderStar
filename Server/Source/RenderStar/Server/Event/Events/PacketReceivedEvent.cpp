#include "RenderStar/Server/Event/Events/PacketReceivedEvent.hpp"

namespace RenderStar::Server::Event::Events
{
    PacketReceivedEvent::PacketReceivedEvent(std::shared_ptr<Network::ClientConnection> conn, Common::Network::IPacket* pkt)
        : connection(std::move(conn))
        , packet(pkt)
    {
    }
}
