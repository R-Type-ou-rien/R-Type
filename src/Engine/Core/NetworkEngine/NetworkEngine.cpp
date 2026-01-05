#include "NetworkEngine.hpp"
#include <cstdint>
#include "../../Lib/Components/NetworkComponents.hpp"

namespace engine {
namespace core {

NetworkEngine::NetworkEngine(NetworkRole role, std::variant<network::Server*, network::Client*> networkInstance)
    : _role(role), _networkInstance(networkInstance) {}

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

        if (std::holds_alternative<network::Server*>(_networkInstance)) {
            auto server = std::get<network::Server*>(_networkInstance);
            msg = server->ReadIncomingMessage();
            if (msg.id != network::GameEvents::NONE)
                hasMsg = true;
        } else {
            auto client = std::get<network::Client*>(_networkInstance);
            msg = client->ReadIncomingMessage();
            if (msg.id != network::GameEvents::NONE)
                hasMsg = true;
        }

        if (!hasMsg)
            break;

        uint32_t guid = msg.msg.header.user_id;

        if (isUdpEvent(msg.id)) {
            if (msg.msg.body.size() < sizeof(uint32_t)) {
                continue;
            }

            uint32_t packetTick = msg.msg.header.tick;

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

bool NetworkEngine::transmitEvent(EventType type, const std::vector<uint8_t>& data, uint32_t tick, uint32_t targetId) {
    try {
        network::message<network::GameEvents> msg;
        msg.header.id = type;
        msg.body = data;
        msg.header.size = data.size();
        msg.header.tick = tick;

        if (std::holds_alternative<network::Server*>(_networkInstance)) {
            auto server = std::get<network::Server*>(_networkInstance);
            server->AddMessageToPlayer(type, targetId, msg);
        } else {
            auto client = std::get<network::Client*>(_networkInstance);
            client->AddMessageToServer(type, msg);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error transmitting event: " << e.what() << std::endl;
        return false;
    }
    return true;
}

}  // namespace core
}  // namespace engine
