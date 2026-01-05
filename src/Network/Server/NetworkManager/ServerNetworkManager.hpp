#pragma once

#include <cstdint>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <utility>

#include "../../Network.hpp"
#include "../../NetworkInterface/message.hpp"

enum class PacketValidation : uint8_t {
    VALID,
    INVALID,
    UNAUTHORIZED,
    INVALID_PAYLOAD_SIZE,
    MALFORMED_PACKET,
    INVALID_EVENT
};

struct PacketValidationResult {
    PacketValidation status;
    std::string errorMessage;

    bool isValid() const { return status == PacketValidation::VALID; }
};

class ServerNetworkManager {
   public:
    ServerNetworkManager();
    ~ServerNetworkManager() = default;

    /**
     * Validates a packet sent from client to server
     * @param event The GameEvent type to validate
     * @param msg The message to validate
     * @return PacketValidationResult containing status and potential error message
     */
    template <typename T>
    PacketValidationResult validateClientPacket(network::GameEvents event, const network::message<T>& msg) const {
        // Check if event is a valid server event (one that server expects to RECEIVE)
        if (!isValidClientEvent(event)) {
            return {PacketValidation::INVALID_EVENT, "Event ID " + std::to_string(static_cast<uint32_t>(event)) +
                                                         " is not a valid event for server to receive"};
        }

        // Check magic value integrity
        if (msg.header.magic_value != MAGIC_VALUE) {
            return {PacketValidation::INVALID, "Invalid magic value in packet header"};
        }

        // Check header size matches body size
        if (msg.header.size != msg.body.size()) {
            return {PacketValidation::INVALID_PAYLOAD_SIZE, "Header size (" + std::to_string(msg.header.size) +
                                                                ") does not match body size (" +
                                                                std::to_string(msg.body.size()) + ")"};
        }

        // Validate payload constraints for specific events
        if (auto payloadValidation = validatePayloadForEvent(event, msg); !payloadValidation.isValid()) {
            return payloadValidation;
        }

        return {PacketValidation::VALID, ""};
    }

    /**
     * Checks if an event is valid for server to RECEIVE (originally sent by CLIENT)
     */
    bool isValidClientEvent(network::GameEvents event) const;

    /**
     * Checks if a server-sent event should use TCP
     */
    bool isTcpEvent(network::GameEvents event) const;

    /**
     * Checks if a server-sent event should use UDP
     */
    bool isUdpEvent(network::GameEvents event) const;

   private:
    std::unordered_set<network::GameEvents> _validClientEvents;  // Events server expects to RECEIVE
    std::unordered_set<network::GameEvents> _tcpEvents;          // Events server SENDS via TCP
    std::unordered_set<network::GameEvents> _udpEvents;          // Events server SENDS via UDP
    std::unordered_map<network::GameEvents, std::pair<size_t, size_t>> _payloadConstraints;

    /**
     * Validates payload size constraints for specific event types
     */
    template <typename T>
    PacketValidationResult validatePayloadForEvent(network::GameEvents event, const network::message<T>& msg) const {
        auto constraints = _payloadConstraints.find(event);
        if (constraints != _payloadConstraints.end()) {
            size_t minSize = constraints->second.first;
            size_t maxSize = constraints->second.second;

            if (msg.body.size() < minSize) {
                return {PacketValidation::INVALID_PAYLOAD_SIZE, "Payload too small for event. Expected at least " +
                                                                    std::to_string(minSize) + " bytes, got " +
                                                                    std::to_string(msg.body.size())};
            }

            if (maxSize > 0 && msg.body.size() > maxSize) {
                return {PacketValidation::INVALID_PAYLOAD_SIZE, "Payload too large for event. Expected at most " +
                                                                    std::to_string(maxSize) + " bytes, got " +
                                                                    std::to_string(msg.body.size())};
            }
        }

        return {PacketValidation::VALID, ""};
    }

    void initializeValidClientEvents();
    void initializeTcpEvents();
    void initializeUdpEvents();
    void initializePayloadConstraints();
};
