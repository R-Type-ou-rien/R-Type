#pragma once

#include <cstdint>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <optional>
#include <utility>

#include "Network.hpp"
#include "NetworkInterface/message.hpp"

enum class PackageValidation : uint8_t {
    VALID,
    INVALID,
    UNAUTHORIZED,
    INVALID_PAYLOAD_SIZE,
    MALFORMED_PACKAGE,
    INVALID_EVENT
};

struct ValidationResult {
    PackageValidation status;
    std::string errorMessage;

    bool isValid() const { return status == PackageValidation::VALID; }
};

class NetworkManager {
   public:
    NetworkManager();
    ~NetworkManager() = default;

    /**
     * Validates a packet sent from client to server
     * @param event The GameEvent type to validate
     * @param msg The message to validate
     * @return ValidationResult containing status and potential error message
     */
    template <typename T>
    ValidationResult validateServerPacket(network::GameEvents event, const network::message<T>& msg) const {
        // Check if event is a valid client event
        if (!isValidClientEvent(event)) {
            return {PackageValidation::INVALID_EVENT,
                    "Event ID " + std::to_string(static_cast<uint32_t>(event)) + " is not a valid client event"};
        }

        // Check magic value integrity
        if (msg.header.magic_value != MAGIC_VALUE) {
            return {PackageValidation::INVALID, "Invalid magic value in packet header"};
        }

        // Check header size matches body size
        if (msg.header.size != msg.body.size()) {
            return {PackageValidation::INVALID_PAYLOAD_SIZE, "Header size (" + std::to_string(msg.header.size) +
                                                                 ") does not match body size (" +
                                                                 std::to_string(msg.body.size()) + ")"};
        }

        // Validate payload constraints for specific events
        if (auto payloadValidation = validatePayloadForEvent(event, msg); !payloadValidation.isValid()) {
            return payloadValidation;
        }

        return {PackageValidation::VALID, ""};
    }

    /**
     * Checks if an event is valid for client to send
     */
    bool isValidClientEvent(network::GameEvents event) const;

    /**
     * Checks if an event should use TCP
     */
    bool isTcpEvent(network::GameEvents event) const;

    /**
     * Checks if an event should use UDP
     */
    bool isUdpEvent(network::GameEvents event) const;

    /**
     * Gets the expected minimum payload size for an event
     */
    std::optional<size_t> getMinPayloadSize(network::GameEvents event) const;

    /**
     * Gets the expected maximum payload size for an event
     */
    std::optional<size_t> getMaxPayloadSize(network::GameEvents event) const;

   private:
    std::unordered_set<network::GameEvents> _validClientEvents;
    std::unordered_set<network::GameEvents> _tcpEvents;
    std::unordered_set<network::GameEvents> _udpEvents;
    std::unordered_map<network::GameEvents, std::pair<size_t, size_t>> _payloadConstraints;

    /**
     * Validates payload size constraints for specific event types
     */
    template <typename T>
    ValidationResult validatePayloadForEvent(network::GameEvents event, const network::message<T>& msg) const {
        auto constraints = _payloadConstraints.find(event);
        if (constraints != _payloadConstraints.end()) {
            size_t minSize = constraints->second.first;
            size_t maxSize = constraints->second.second;

            if (msg.body.size() < minSize) {
                return {PackageValidation::INVALID_PAYLOAD_SIZE, "Payload too small for event. Expected at least " +
                                                                     std::to_string(minSize) + " bytes, got " +
                                                                     std::to_string(msg.body.size())};
            }

            if (maxSize > 0 && msg.body.size() > maxSize) {
                return {PackageValidation::INVALID_PAYLOAD_SIZE, "Payload too large for event. Expected at most " +
                                                                     std::to_string(maxSize) + " bytes, got " +
                                                                     std::to_string(msg.body.size())};
            }
        }

        return {PackageValidation::VALID, ""};
    }

    void initializeValidClientEvents();
    void initializeTcpEvents();
    void initializeUdpEvents();
    void initializePayloadConstraints();
};