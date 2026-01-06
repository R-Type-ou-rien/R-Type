#include "NetworkEngine.hpp"
#include <cstdint>
#include <memory>  // Added for std::shared_ptr
#include "../../Lib/Components/NetworkComponents.hpp"

namespace engine {
namespace core {

NetworkEngine::NetworkEngine(NetworkRole role, std::string host, uint16_t port, int timeout) : _role(role) {
    if (role == NetworkRole::SERVER) {
        _networkInstance = std::make_shared<network::Server>(port, timeout);
        std::get<std::shared_ptr<network::Server>>(_networkInstance)->Start();
    } else {
        _networkInstance = std::make_shared<network::Client>(host, port);
    }
}

bool NetworkEngine::isUdpEvent(EventType type) {
    switch (type) {
        case network::GameEvents::C_INPUT:
        case network::GameEvents::S_SNAPSHOT:
        case network::GameEvents::C_VOICE_PACKET:
        case network::GameEvents::S_VOICE_RELAY:
        case network::GameEvents::S_SCORE_UPDATE:
        case network::GameEvents::S_PLAYER_DEATH:
            return true;
        default:
            return false;
    }
}

void NetworkEngine::processIncomingPackets(uint32_t tick) {
    while (true) {
        network::coming_message msg;
        bool hasMsg = false;

        if (std::holds_alternative<std::shared_ptr<network::Server>>(_networkInstance)) {
            auto server = std::get<std::shared_ptr<network::Server>>(_networkInstance);
            msg = server->ReadIncomingMessage();
            if (msg.id != network::GameEvents::NONE)
                hasMsg = true;
        } else {
            auto client = std::get<std::shared_ptr<network::Client>>(_networkInstance);
            msg = client->ReadIncomingMessage();
            if (msg.id != network::GameEvents::NONE)
                hasMsg = true;
        }

        if (!hasMsg)
            break;

        uint32_t guid = msg.msg.header.user_id;

        if (isUdpEvent(msg.id)) {
            // Check if body is large enough to contain GUID (at least 4 bytes)
            if (msg.msg.body.size() < sizeof(uint32_t)) {
                continue;
            }

            uint32_t packetTick = msg.msg.header.tick;

            // User requested casting to ComponentPacket
            // We interpret the start of the body as the struct.
            // Note: This assumes the struct layout matches the sender's serialization.
            ComponentPacket* packet = reinterpret_cast<ComponentPacket*>(msg.msg.body.data());
            uint32_t guid = packet->entity_guid;
            if (packetTick <= _lastPacketTickMap[guid]) {
                continue;
            }
            _lastPacketTickMap[guid] = packetTick;
            _processedEvents[msg.id].push_back(msg.msg.body);

        } else {
            _processedEvents[msg.id].push_back(msg.msg.body);
        }
    }
}

std::map<NetworkEngine::EventType, std::vector<std::vector<uint8_t>>> NetworkEngine::getPendingEvents() {
    auto events = _processedEvents;
    _processedEvents.clear();
    return events;
}

void NetworkEngine::setPort(uint16_t port) {}

void NetworkEngine::setTimeout(int timeout) {}

}  // namespace core
}  // namespace engine
