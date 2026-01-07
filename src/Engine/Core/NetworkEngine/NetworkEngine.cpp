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
            _processedEvents[msg.id] = msg.msg;

        } else {
            _processedEvents[msg.id] = msg.msg;
        }
    }
}

std::map<NetworkEngine::EventType, network::message<NetworkEngine::EventType>> NetworkEngine::getPendingEvents() {
    auto events = _processedEvents;
    _processedEvents.clear();
    return events;
}

void NetworkEngine::setTimeout(int timeout) {
    if (std::holds_alternative<std::shared_ptr<network::Server>>(_networkInstance)) {
        auto server = std::get<std::shared_ptr<network::Server>>(_networkInstance);
        server->setTimeout(timeout);
    }
}

}  // namespace core
}  // namespace engine
