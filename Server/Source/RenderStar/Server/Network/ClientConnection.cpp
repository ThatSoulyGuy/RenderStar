#include "RenderStar/Server/Network/ClientConnection.hpp"

namespace RenderStar::Server::Network
{
    ClientConnection::ClientConnection(SocketPointer tcpSocket)
        : socket(std::move(tcpSocket))
        , receiveBuffer(RECEIVE_BUFFER_SIZE)
    {
        if (socket && socket->is_open())
        {
            asio::error_code errorCode;
            auto endpoint = socket->remote_endpoint(errorCode);
            if (!errorCode)
                remoteAddress = endpoint.address().to_string() + ":" + std::to_string(endpoint.port());
        }
    }

    bool ClientConnection::IsConnected() const
    {
        return socket && socket->is_open();
    }

    void ClientConnection::Close()
    {
        if (socket && socket->is_open())
        {
            asio::error_code errorCode;
            socket->shutdown(asio::ip::tcp::socket::shutdown_both, errorCode);
            socket->close(errorCode);
        }
    }
}
