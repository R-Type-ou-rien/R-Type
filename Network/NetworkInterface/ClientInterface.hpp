
#pragma once
#include "Connection.hpp"
#include "MsgQueue.hpp"

namespace network {
template <typename T>
class ClientInterface {
   public:
    ClientInterface() : _socketUDP(_context) {}

    virtual ~ClientInterface() { Disconnect(); }

   public:
    bool Connect(const std::string& host, const uint16_t port) {
        try {
            asio::ip::tcp::resolver resolver(_context);
            asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

            _connection = std::make_unique<Connection<T>>(Connection<T>::owner::client, _context,
                                                          asio::ip::tcp::socket(_context), _qMessagesIn, _socketUDP);
            _connection->ConnectToServer(endpoints);

            _serverUDPEndpoint = asio::ip::udp::endpoint(endpoints.begin()->endpoint().address(), port);
            std::cout << "[CLIENT_DEBUG] Resolved Server UDP Endpoint: " << _serverUDPEndpoint << "\n";
            _connection->SetUDPEndpoint(_serverUDPEndpoint);

            _socketUDP.open(asio::ip::udp::v4());
            _socketUDP.bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), 0));
            std::cout << "[CLIENT_DEBUG] UDP Socket bound to local port: " << _socketUDP.local_endpoint().port()
                      << "\n";

            ReceiveUDP();

            thrContext = std::thread([this]() {
                try {
                    _context.run();
                } catch (const std::exception& e) {
                    std::cerr << "[CLIENT] IO Thread Exception: " << e.what() << std::endl;
                }
            });
        } catch (std::exception& e) {
            std::cerr << "Client Exception: " << e.what() << "\n";
            return false;
        }
        return true;
    }

    void Disconnect() {
        if (IsConnected()) {
            _connection->Disconnect();
        }

        _context.stop();
        if (thrContext.joinable())
            thrContext.join();

        _connection.release();
    }

    bool IsConnected() {
        if (_connection)
            return _connection->IsConnected();
        else
            return false;
    }

   public:
    void Send(const message<T>& msg) {
        if (IsConnected())
            _connection->Send(msg);
    }

    void SendUdp(const message<T>& msg) {
        if (IsConnected())
            _connection->SendUdp(msg);
    }

    void ReceiveUDP() {
        if (_udpMsgTemporaryIn.size() < 4096)
            _udpMsgTemporaryIn.resize(4096);
        _socketUDP.async_receive_from(
            asio::buffer(_udpMsgTemporaryIn.data(), _udpMsgTemporaryIn.size()), _serverUDPEndpoint,
            [this](std::error_code ec, std::size_t len) {
                if (!ec && len > 0) {
                    message<T> msg;

                    if (len >= sizeof(message_header<T>)) {
                        std::memcpy(&msg.header, _udpMsgTemporaryIn.data(), sizeof(message_header<T>));

                        if (msg.header.magic_value != MAGIC_VALUE) {
                            std::cout << "[UDP] Error: Invalid Magic Value " << std::hex << msg.header.magic_value
                                      << std::dec << "\n";
                        } else {
                            if (msg.header.size > 0) {
                                if (msg.header.size <= len - sizeof(message_header<T>)) {
                                    msg.body.resize(msg.header.size);
                                    std::memcpy(msg.body.data(), _udpMsgTemporaryIn.data() + sizeof(message_header<T>),
                                                msg.header.size);
                                    _qMessagesIn.push_back({nullptr, msg});
                                } else {
                                    std::cout << "[UDP] Error: Invalid Size " << msg.header.size << " (Len: " << len
                                              << ")\n";
                                }
                            } else {
                                _qMessagesIn.push_back({nullptr, msg});
                            }
                        }
                    }

                    ReceiveUDP();
                } else if (ec) {
                    std::cout << "[UDP] Reception Error: " << ec.message() << "\n";
                }
            });
    }

    MsgQueue<owned_message<T>>& Incoming() { return _qMessagesIn; }

   protected:
    asio::io_context _context;
    std::thread thrContext;
    std::unique_ptr<Connection<T>> _connection;

    asio::ip::udp::socket _socketUDP;
    asio::ip::udp::endpoint _serverUDPEndpoint;
    std::vector<uint8_t> _udpMsgTemporaryIn;

    MsgQueue<owned_message<T>> _qMessagesIn;
};
}  // namespace network
