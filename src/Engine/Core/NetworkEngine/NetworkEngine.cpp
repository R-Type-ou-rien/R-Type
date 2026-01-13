#include "NetworkEngine.hpp"
#include <cstdint>
#include <memory>
#include <vector>
#include <iostream>
#include <string>
#include <map>
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
    if (std::holds_alternative<std::shared_ptr<network::Server>>(_networkInstance)) {
        auto server = std::get<std::shared_ptr<network::Server>>(_networkInstance);
        server->Update(-1, false);
    }

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

        uint32_t user_guid = msg.msg.header.user_id;

        if (isUdpEvent(msg.id)) {
            uint32_t packetTick = msg.msg.header.tick;
            uint32_t sequence_guid = user_guid;

            if (msg.id == network::GameEvents::S_SNAPSHOT) {
                auto temp_msg = msg.msg;
                ComponentPacket temp_packet;
                temp_msg >> temp_packet;
                sequence_guid = temp_packet.entity_guid;

                if (_lastPacketTickMap.count(sequence_guid) && packetTick < _lastPacketTickMap[sequence_guid]) {
                    continue;
                }
                _lastPacketTickMap[sequence_guid] = packetTick;
            }
            _processedEvents[msg.id].push_back(msg.msg);

        } else {
            _processedEvents[msg.id].push_back(msg.msg);
        }
    }
}

std::map<NetworkEngine::EventType, std::vector<network::message<NetworkEngine::EventType>>>
NetworkEngine::getPendingEvents() {
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

uint32_t NetworkEngine::getClientId() const {
    if (std::holds_alternative<std::shared_ptr<network::Client>>(_networkInstance)) {
        auto client = std::get<std::shared_ptr<network::Client>>(_networkInstance);
        return client->getId();
    }
    return 0;
}

}  // namespace core
}  // namespace engine
