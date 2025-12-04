#pragma once

#include "Connection.hpp"
#include "MsgQueue.hpp"
#include "NetworkCommon.hpp"
#include "message.hpp"

namespace network {
template <typename T>
class ServerInterface {
   public:
    ServerInterface(uint16_t port)
        : asioAcceptor(_asioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)), _port(port) {}

    virtual ~ServerInterface() { Stop(); }

    bool Start() {
        try {
            _socketUDP.open(asio::ip::udp::v4());
            _socketUDP.bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), _port));
            WaitForClientConnection();
            ReceiveUDP();

            _threadContext = std::thread([this]() { _asioContext.run(); });
        } catch (std::exception& e) {
            std::cerr << "[SERVER] Exception: " << e.what() << "\n";
            return false;
        }

        std::cout << "[SERVER] Started!\n";
        return true;
    }

    void Stop() {
        _asioContext.stop();

        if (_threadContext.joinable())
            _threadContext.join();

        std::cout << "[SERVER] Stopped!\n";
    }

    void WaitForClientConnection() {
        asioAcceptor.async_accept([this](std::error_code ec, asio::ip::tcp::socket socket) {
            if (!ec) {
                std::cout << "[SERVER] New Connection: " << socket.remote_endpoint() << "\n";

                std::shared_ptr<Connection<T>> newconn = std::make_shared<Connection<T>>(
                    Connection<T>::owner::server, _asioContext, std::move(socket), _MessagesIn, _socketUDP);

                if (OnClientConnect(newconn)) {
                    _deqConnections.push_back(std::move(newconn));

                    _deqConnections.back()->ConnectToClient(nIDCounter++);

                    std::cout << "[" << _deqConnections.back()->GetID() << "] Connection Approved\n";
                } else {
                    std::cout << "[-----] Connection Denied\n";
                }
            } else {
                std::cout << "[SERVER] New Connection Error: " << ec.message() << "\n";
            }

            WaitForClientConnection();
        });
    }

    void MessageClient(std::shared_ptr<Connection<T>> client, const message<T>& msg) {
        if (client && client->IsConnected()) {
            client->Send(msg);
        } else {
            OnClientDisconnect(client);

            client.reset();

            _deqConnections.erase(std::remove(_deqConnections.begin(), _deqConnections.end(), client),
                                  _deqConnections.end());
        }
    }

    void MessageAllClients(const message<T>& msg, std::shared_ptr<Connection<T>> pIgnoreClient = nullptr) {
        bool bInvalidClientExists = false;

        for (auto& client : _deqConnections) {
            if (client && client->IsConnected()) {
                if (client != pIgnoreClient)
                    client->Send(msg);
            } else {
                OnClientDisconnect(client);
                client.reset();

                bInvalidClientExists = true;
            }
        }

        if (bInvalidClientExists)
            _deqConnections.erase(std::remove(_deqConnections.begin(), _deqConnections.end(), nullptr),
                                  _deqConnections.end());
    }

    void MessageClientUDP(std::shared_ptr<Connection<T>> client, const message<T>& msg) {
        if (client && client->IsConnected()) {
            client->SendUdp(msg);
        } else {
            OnClientDisconnect(client);

            client.reset();

            _deqConnections.erase(std::remove(_deqConnections.begin(), _deqConnections.end(), client),
                                  _deqConnections.end());
        }
    }

    void MessageAllClientsUDP(const message<T>& msg, std::shared_ptr<Connection<T>> pIgnoreClient = nullptr) {
        bool bInvalidClientExists = false;

        for (auto& client : _deqConnections) {
            if (client && client->IsConnected()) {
                if (client != pIgnoreClient)
                    client->SendUdp(msg);
            } else {
                OnClientDisconnect(client);
                client.reset();

                bInvalidClientExists = true;
            }
        }

        if (bInvalidClientExists)
            _deqConnections.erase(std::remove(_deqConnections.begin(), _deqConnections.end(), nullptr),
                                  _deqConnections.end());
    }

    void Update(size_t nMaxMessages = -1, bool bWait = false) {
        if (bWait)
            _MessagesIn.wait();

        size_t nMessageCount = 0;
        while (nMessageCount < nMaxMessages && !_MessagesIn.empty()) {
            auto msg = _MessagesIn.pop_front();

            OnMessage(msg.remote, msg.msg);

            nMessageCount++;
        }
    }

    void ReceiveUDP() {
        _socketUDP.async_receive_from(
            asio::buffer(_udpMsgTemporaryIn), _udpEndpointTemporary, [this](std::error_code ec, std::size_t len) {
                if (!ec && len > 0) {
                    network::message<T> msg;

                    if (len >= sizeof(network::message_header<T>)) {
                        std::memcpy(&msg.header, _udpMsgTemporaryIn, sizeof(network::message_header<T>));

                        if (msg.header.size > 0) {
                            if (len >= sizeof(network::message_header<T>) + msg.header.size) {
                                msg.body.resize(msg.header.size);
                                std::memcpy(msg.body.data(),
                                            _udpMsgTemporaryIn.data() + sizeof(network::message_header<T>),
                                            msg.header.size);
                            } else {
                                std::cout << "[UDP] Erreur : Paquet corrompu reçu.\n";
                                ReceiveUDP();
                                return;
                            }
                        }

                        std::shared_ptr<connection<T>> pClient = nullptr;
                        bool bClientFound = false;

                        for (auto& client : _deqConnections) {
                            if (client->IsConnected() && client->m_udpRemoteEndpoint == _udpEndpointTemporary) {
                                pClient = client;
                                bClientFound = true;
                                break;
                            }
                            if (client.id == msg.header.id) {
                                pClient = client;
                                bClientFound = true;
                                break;
                            }
                        }

                        if (bClientFound) {
                            _MessagesIn.push_back({pClient, msg});
                        } else {
                            std::cout << "[UDP] Erreur : Paquet reçu d'un client inconnu.\n";
                        }
                    }
                } else if (ec) {
                    std::cout << "[UDP] Erreur réception : " << ec.message() << "\n";
                }
                ReceiveUDP();
            });
    }

   protected:
    virtual bool OnClientConnect(std::shared_ptr<Connection<T>> client) { return false; }

    virtual void OnClientDisconnect(std::shared_ptr<Connection<T>> client) {}

    virtual void OnMessage(std::shared_ptr<Connection<T>> client, message<T>& msg) {}

   protected:
    MsgQueue<owned_message<T>> _MessagesIn;

    std::vector<uint8_t> _udpMsgTemporaryIn;
    MsgQueue<owned_message<T>> _UDPMessageIn;
    asio::ip::udp::endpoint _udpEndpointTemporary;

    std::deque<std::shared_ptr<Connection<T>>> _deqConnections;

    asio::io_context _asioContext;
    std::thread _threadContext;

    asio::ip::tcp::acceptor asioAcceptor;
    asio::ip::udp::socket _socketUDP;

    uint32_t nIDCounter = 10000;

   private:
    uint16_t _port;
};
}  // namespace network
