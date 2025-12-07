
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

            _connection =
                std::make_unique<Connection<T>>(Connection<T>::owner::client, _context, asio::ip::tcp::socket(_context),
                                                _qMessagesIn, _serverUDPEndpoint);

            _connection->ConnectToServer(endpoints);

            _serverUDPEndpoint = asio::ip::udp::endpoint(endpoints.begin()->endpoint().address(), port);

            _socketUDP.open(asio::ip::udp::v4());

            ReceiveUDP();

            thrContext = std::thread([this]() { _context.run(); });
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
        _socketUDP.async_receive_from(
            asio::buffer(_udpMsgTemporaryIn), _serverUDPEndpoint, [this](std::error_code ec, std::size_t len) {
                if (!ec && len > 0) {
                    message<T> msg;

                    if (len >= sizeof(message_header<T>)) {
                        std::memcpy(&msg.header, _udpMsgTemporaryIn.data(), sizeof(message_header<T>));

                        if (msg.header.size > 0 && len >= sizeof(message_header<T>) + msg.header.size) {
                            msg.body.resize(msg.header.size);
                            std::memcpy(msg.body.data(), _udpMsgTemporaryIn.data() + sizeof(message_header<T>),
                                        msg.header.size);
                        }

                        _qMessagesIn.push_back({nullptr, msg});
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
