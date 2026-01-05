#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include <variant>
#include "../../../Network/Network.hpp"
#include "../../../Network/Server/Server.hpp"
#include "../../../Network/Client/Client.hpp"

namespace engine {
namespace core {

class NetworkEngine {
   public:
    enum class NetworkRole { CLIENT, SERVER };

    using EventType = network::GameEvents;

    NetworkEngine(NetworkRole role, uint16_t port = 0, int timeout = 30);
    ~NetworkEngine() = default;

    bool transmitEvent(EventType type, const std::vector<uint8_t>& data, uint32_t tick = -1, uint32_t targetId = -1);
    void processIncomingPackets(uint32_t tick);
    std::map<EventType, std::vector<std::vector<uint8_t>>> getPendingEvents();

   private:
    NetworkRole _role;
    std::variant<std::shared_ptr<network::Server>, std::shared_ptr<network::Client>> _networkInstance;

    std::map<uint32_t, uint32_t> _lastPacketTickMap;
    std::map<EventType, std::vector<std::vector<uint8_t>>> _processedEvents;

    bool isUdpEvent(EventType type);
};

}  // namespace core
}  // namespace engine
