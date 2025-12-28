#pragma once

#include "MsgQueue.hpp"
#include "NetworkCommon.hpp"
#include "message.hpp"

namespace network {
template <typename T>
class Connection : public std::enable_shared_from_this<Connection<T>> {
   public:
    enum class owner { server, client };

   public:
    Connection(owner parent, asio::io_context& asioContext, asio::ip::tcp::socket socket,
               MsgQueue<owned_message<T>>& In, asio::ip::udp::socket& udpSocket)
        : _asioContext(asioContext),
          _socket(std::move(socket)),
          _qMessagesIn(In),
          _udpSocket(udpSocket),
          m_timerTimeout(asioContext) {
        _OwnerType = parent;
    }

    virtual ~Connection() {}

    uint32_t GetID() const { return id; }

   public:
    void ConnectToClient(uint32_t uid = 0) {
        if (_OwnerType == owner::server) {
            if (_socket.is_open()) {
                id = uid;
                ReadHeader();
            }
        }
    }

    void ConnectToServer(const asio::ip::tcp::resolver::results_type& endpoints) {
        if (_OwnerType == owner::client) {
            asio::async_connect(_socket, endpoints, [this](std::error_code ec, asio::ip::tcp::endpoint endpoint) {
                if (!ec) {
                    ReadHeader();
                }
            });
        }
    }

    void Disconnect() {
        if (IsConnected())
            asio::post(_asioContext, [this]() { _socket.close(); });
    }

    bool IsConnected() const { return _socket.is_open(); }

    void StartListening() {}

   public:
    void Send(const message<T>& msg) {
        asio::post(_asioContext, [this, msg]() mutable {
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            msg.to_little_endian();
#endif
            bool WritingMessage = !_qMessagesOut.empty();
            _qMessagesOut.push_back(msg);
            if (!WritingMessage) {
                WriteHeader();
            }
        });
    }

    void SendUdp(const message<T>& msg) {
        asio::post(_asioContext, [this, msg]() mutable {
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            msg.to_little_endian();
#endif
            bool WritingMessage = !_udpMessagesOut.empty();
            _udpMessagesOut.push_back(msg);
            if (!WritingMessage) {
                WriteUDP();
            }
        });
    }

   protected:
    void WriteHeader() {
        asio::async_write(_socket, asio::buffer(&_qMessagesOut.front().header, sizeof(message_header<T>)),
                          [this](std::error_code ec, std::size_t length) {
                              if (!ec) {
                                  if (_qMessagesOut.front().body.size() > 0) {
                                      WriteBody();
                                  } else {
                                      _qMessagesOut.pop_front();
                                      if (!_qMessagesOut.empty()) {
                                          WriteHeader();
                                      }
                                  }
                              } else {
                                  std::cout << "[" << id << "] Write Header Fail.\n";
                                  std::cout << ec.message() << "\n";
                                  _socket.close();
                              }
                          });
    }

    void WriteBody() {
        asio::async_write(_socket, asio::buffer(_qMessagesOut.front().body.data(), _qMessagesOut.front().body.size()),
                          [this](std::error_code ec, std::size_t length) {
                              if (!ec) {
                                  _qMessagesOut.pop_front();
                                  if (!_qMessagesOut.empty()) {
                                      WriteHeader();
                                  }
                              } else {
                                  std::cout << "[" << id << "] Write Body Fail.\n";
                                  std::cout << ec.message() << "\n";
                                  _socket.close();
                              }
                          });
    }

    void WriteUDP() {
        message<T> msg = _udpMessagesOut.pop_front();
        std::vector<uint8_t> Buffer;
        Buffer.resize(sizeof(msg.header) + msg.body.size());

        std::memcpy(Buffer.data(), &msg.header, sizeof(msg.header));
        if (!msg.body.empty())
            std::memcpy(Buffer.data() + sizeof(msg.header), msg.body.data(), msg.body.size());
        _udpSocket.async_send_to(asio::buffer(Buffer.data(), Buffer.size()), _udpRemoteEndpoint,
                                 [this, Buffer = std::move(Buffer)](std::error_code ec, std::size_t bytes_sent) {
                                     if (!ec) {
                                         if (!_udpMessagesOut.empty()) {
                                             WriteUDP();
                                         }
                                     } else {
                                         std::cout << "[" << id << "] Write UDP Fail.\n";
                                         std::cout << ec.message() << "\n";
                                         _socket.close();
                                     }
                                 });
    }

    void ReadHeader() {
        asio::async_read(_socket, asio::buffer(&_msgTemporaryIn.header, sizeof(message_header<T>)),
                         [this](std::error_code ec, std::size_t length) {
                             if (!ec) {
                                 if (_msgTemporaryIn.header.magic_value != MAGIC_VALUE) {
                                     std::cout << "[" << id << "] Error: Invalid Magic Value " << std::hex
                                               << _msgTemporaryIn.header.magic_value << std::dec << "\n";
                                     _socket.close();
                                     return;
                                 }
                                 if (_msgTemporaryIn.header.size > 0) {
                                     _msgTemporaryIn.body.resize(_msgTemporaryIn.header.size);
                                     ReadBody();
                                 } else {
                                     AddToIncomingMessageQueue();
                                 }
                             } else {
                                 std::cout << "[" << id << "] Read Header Fail.\n";
                                 std::cout << ec.message() << "\n";
                                 _socket.close();
                             }
                         });
    }

    void ReadBody() {
        asio::async_read(_socket, asio::buffer(_msgTemporaryIn.body.data(), _msgTemporaryIn.body.size()),
                         [this](std::error_code ec, std::size_t length) {
                             if (!ec) {
                                 AddToIncomingMessageQueue();
                             } else {
                                 std::cout << "[" << id << "] Read Body Fail.\n";
                                 std::cout << ec.message() << "\n";
                                 _socket.close();
                             }
                         });
    }

    void AddToIncomingMessageQueue() {
        if (_OwnerType == owner::server)
            _qMessagesIn.push_back({this->shared_from_this(), _msgTemporaryIn});
        else
            _qMessagesIn.push_back({nullptr, _msgTemporaryIn});

        ReadHeader();
    }

    void ResetTimeout() {
        m_timerTimeout.cancel();

        if (m_nTimeoutDuration.count() == 0)
            return;

        m_timerTimeout.expires_after(m_nTimeoutDuration);

        m_timerTimeout.async_wait([this](const std::error_code& ec) {
            if (ec == asio::error::operation_aborted)
                return;
            if (!ec) {
                std::cout << "[TIMEOUT] Client " << id << " n'a pas répondu depuis " << m_nTimeoutDuration.count()
                          << "s.\n";
                Disconnect();
            }
        });
    }

   public:
    void SetUDPEndpoint(asio::ip::udp::endpoint endpoint) { _udpRemoteEndpoint = endpoint; }
    asio::ip::udp::endpoint GetUDPEndpoint() const { return _udpRemoteEndpoint; }
    void SetTimeout(int seconds) {
        m_nTimeoutDuration = std::chrono::seconds(seconds);
        ResetTimeout();
    }

   protected:
    asio::ip::udp::endpoint _udpRemoteEndpoint;
    asio::ip::udp::socket& _udpSocket;

    asio::ip::tcp::socket _socket;

    asio::io_context& _asioContext;

    MsgQueue<message<T>> _qMessagesOut;
    MsgQueue<message<T>> _udpMessagesOut;

    MsgQueue<owned_message<T>>& _qMessagesIn;

    message<T> _msgTemporaryIn;

    owner _OwnerType = owner::server;

    uint32_t id = 0;

    unsigned int SequenceID = 0;

    asio::steady_timer m_timerTimeout;
    std::chrono::seconds m_nTimeoutDuration = std::chrono::seconds(0);
};
}  // namespace network
