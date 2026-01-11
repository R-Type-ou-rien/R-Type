#pragma once

#include "Connection.hpp"
#include "MsgQueue.hpp"
#include "message.hpp"
#ifdef _WIN32
#include <winsock2.h>
#include <iphlpapi.h>
#include <stdio.h>
#pragma comment(lib, "IPHLPAPI.lib")
#else
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

namespace network {
template <typename T>
class ServerInterface {
   public:
    ServerInterface(uint16_t port)
        : asioAcceptor(_asioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
          _socketUDP(_asioContext),
          _port(port) {}

    virtual ~ServerInterface() { Stop(); }

    bool Start() {
        std::cout << "[SERVER] Starting server on port 4040..." << std::endl;
        std::cout << "[SERVER] Finding LAN IP addresses...\n";

#ifdef _WIN32
        ULONG outBufLen = 15000;
        PIP_ADAPTER_INFO pAdapterInfo = (IP_ADAPTER_INFO*)malloc(outBufLen);
        if (pAdapterInfo == NULL)
            return false;

        if (GetAdaptersInfo(pAdapterInfo, &outBufLen) == NO_ERROR) {
            PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
            while (pAdapter) {
                std::string ip(pAdapter->IpAddressList.IpAddress.String);
                if (ip != "127.0.0.1" && ip != "0.0.0.0") {
                    std::cout << "[SERVER] LAN IP: " << ip << " (Interface: " << pAdapter->Description << ")\n";
                }
                pAdapter = pAdapter->Next;
            }
        }
        free(pAdapterInfo);
#else
        struct ifaddrs* ifAddrStruct = NULL;
        struct ifaddrs* ifa = NULL;
        void* tmpAddrPtr = NULL;

        getifaddrs(&ifAddrStruct);

        bool found = false;
        if (!ifAddrStruct) {
            return false;
        }
        for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
            if (!ifa->ifa_addr) {
                continue;
            }
            if (ifa->ifa_addr->sa_family == AF_INET) {
                tmpAddrPtr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
                char addressBuffer[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
                std::string ip(addressBuffer);
                if (ip != "127.0.0.1") {
                    std::cout << "[SERVER] LAN IP: " << ip << " (Interface: " << ifa->ifa_name << ")\n";
                    found = true;
                    break;
                }
            }
        }
        if (ifAddrStruct != NULL)
            freeifaddrs(ifAddrStruct);

        if (!found) {
            std::cout << "[SERVER] Could not detect LAN IP. Please check `ip addr` or `ifconfig`.\n";
        }
#endif

        try {
            _socketUDP.open(asio::ip::udp::v4());
            _socketUDP.bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), _port));
            WaitForClientConnection();
            ReceiveUDP();

            _threadContext = std::thread([this]() {
                try {
                    _asioContext.run();
                } catch (std::exception& e) {
                    std::cerr << "[SERVER] Thread Exception: " << e.what() << "\n";
                }
            });
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

                newconn->ConnectToClient(nIDCounter++);

                _deqConnections.push_back(newconn);
                if (OnClientConnect(newconn)) {
                    std::cout << "[" << _deqConnections.back()->GetID() << "] Connection Approved\n";
                } else {
                    _deqConnections.pop_back();
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

        bool bInvalidClientExists = false;
        for (auto& client : _deqConnections) {
            if (!client->IsConnected()) {
                OnClientDisconnect(client);
                client.reset();
                bInvalidClientExists = true;
                continue;
            }
            // client->ResetTimeout();
        }

        if (bInvalidClientExists) {
            _deqConnections.erase(std::remove(_deqConnections.begin(), _deqConnections.end(), nullptr),
                                  _deqConnections.end());
        }
    }

    virtual void ReceiveUDP() {
        if (_udpMsgTemporaryIn.size() < 4096)
            _udpMsgTemporaryIn.resize(4096);
        _socketUDP.async_receive_from(
            asio::buffer(_udpMsgTemporaryIn.data(), _udpMsgTemporaryIn.size()), _udpEndpointTemporary,
            [this](std::error_code ec, std::size_t len) {
                if (!ec && len > 0) {
                    std::cout << "[DEBUG_UDP] Packet Received of size " << len << "\n";
                    network::message<T> msg;

                    if (len >= sizeof(network::message_header<T>)) {
                        std::memcpy(&msg.header, _udpMsgTemporaryIn.data(), sizeof(network::message_header<T>));
                        std::cout << "[DEBUG_UDP] Header ID: " << msg.header.user_id << "\n";

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

                        std::shared_ptr<Connection<T>> pClient = nullptr;
                        bool bClientFound = false;

                        for (auto& client : _deqConnections) {
                            if (client->IsConnected() && client->GetUDPEndpoint() == _udpEndpointTemporary) {
                                pClient = client;
                                bClientFound = true;
                                break;
                            }
                            if (client->GetID() == msg.header.user_id) {
                                pClient = client;
                                pClient->SetUDPEndpoint(_udpEndpointTemporary);
                                std::cout << "[UDP] Client " << client->GetID()
                                          << " endpoint confirmed: " << _udpEndpointTemporary << "\n";
                                bClientFound = true;
                                break;
                            }
                        }

                        if (bClientFound) {
                            std::cout << "[UDP] Client Found! Pushing to queue.\n";
                            _MessagesIn.push_back({pClient, msg});
                        } else {
                            std::cout << "[UDP] Erreur : Paquet reçu d'un client inconnu (ID: " << msg.header.user_id
                                      << ").\n";
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
