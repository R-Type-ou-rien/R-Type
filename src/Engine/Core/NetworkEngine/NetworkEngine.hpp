#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include <variant>
#include <string>
#include "../../../Network/Network.hpp"
#include "../../../Network/Server/Server.hpp"
#include "../../../Network/Client/Client.hpp"

namespace engine {
namespace core {

class NetworkEngine {
   public:
    enum class NetworkRole { CLIENT, SERVER };

    using EventType = network::GameEvents;

    NetworkEngine(NetworkRole role, std::string host = "127.0.0.1", uint16_t port = 4040, int timeout = 15);
    ~NetworkEngine() = default;

    template <typename Data>
    bool transmitEvent(EventType type, const Data& data, uint32_t tick, uint32_t targetId = 0) {
        try {
            network::message<network::GameEvents> msg;
            msg.header.id = type;
            msg.header.tick = tick;
            msg << data;
            msg.header.size = msg.body.size();

            if (std::holds_alternative<std::shared_ptr<network::Server>>(_networkInstance)) {
                auto server = std::get<std::shared_ptr<network::Server>>(_networkInstance);
                server->AddMessageToPlayer(type, targetId, msg);
            } else {
                auto client = std::get<std::shared_ptr<network::Client>>(_networkInstance);
                client->AddMessageToServer(type, msg);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error transmitting event: " << e.what() << std::endl;
            return false;
        }
        return true;
    }

    void setTimeout(int timeout);
    void processIncomingPackets(uint32_t tick);
    std::map<EventType, std::vector<network::message<EventType>>> getPendingEvents();
    uint32_t getClientId() const;

    std::variant<std::shared_ptr<network::Server>, std::shared_ptr<network::Client>>& getNetworkInstance() {
        return _networkInstance;
    }

   private:
    NetworkRole _role;
    std::variant<std::shared_ptr<network::Server>, std::shared_ptr<network::Client>> _networkInstance;

    std::map<uint32_t, uint32_t> _lastPacketTickMap;
    std::map<EventType, std::vector<network::message<EventType>>> _processedEvents;

    bool isUdpEvent(EventType type);
};

}  // namespace core
}  // namespace engine
