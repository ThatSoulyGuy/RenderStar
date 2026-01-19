#pragma once

#include <asio.hpp>
#include <memory>
#include <string>
#include <vector>

namespace RenderStar::Server::Network
{
    struct ClientConnection
    {
        using TcpSocket = asio::ip::tcp::socket;
        using SocketPointer = std::shared_ptr<TcpSocket>;

        SocketPointer socket;
        std::string remoteAddress;
        std::vector<uint8_t> receiveBuffer;

        static constexpr size_t RECEIVE_BUFFER_SIZE = 65536;

        explicit ClientConnection(SocketPointer tcpSocket);

        bool IsConnected() const;

        void Close();
    };
}
