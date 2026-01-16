#include <gtest/gtest.h>
#include <string>
#include "NetworkManager.hpp"

class NetworkManagerTest : public ::testing::Test {
   protected:
    NetworkManager manager;

    template <typename T>
    network::message<network::GameEvents> createValidMessage(network::GameEvents event, const T& data) {
        network::message<network::GameEvents> msg;
        msg << data;
        msg.header.id = event;
        msg.header.magic_value = MAGIC_VALUE;
        msg.header.size = msg.body.size();
        return msg;
    }

    template <typename T>
    network::message<network::GameEvents> createInvalidMagicMessage(network::GameEvents event, const T& data) {
        auto msg = createValidMessage(event, data);
        msg.header.magic_value = 0xDEADC0DE;  // Wrong magic
        return msg;
    }

    template <typename T>
    network::message<network::GameEvents> createSizeMismatchMessage(network::GameEvents event, const T& data) {
        auto msg = createValidMessage(event, data);
        msg.header.size = msg.body.size() + 100;  // Wrong size
        return msg;
    }
};

TEST_F(NetworkManagerTest, ValidClientEvents_AreRecognized) {
    EXPECT_TRUE(manager.isValidClientEvent(network::GameEvents::C_PING_SERVER));
    EXPECT_TRUE(manager.isValidClientEvent(network::GameEvents::C_LOGIN));
    EXPECT_TRUE(manager.isValidClientEvent(network::GameEvents::C_REGISTER));
    EXPECT_TRUE(manager.isValidClientEvent(network::GameEvents::C_INPUT));
    EXPECT_TRUE(manager.isValidClientEvent(network::GameEvents::C_DISCONNECT));
    EXPECT_TRUE(manager.isValidClientEvent(network::GameEvents::C_JOIN_ROOM));
    EXPECT_TRUE(manager.isValidClientEvent(network::GameEvents::C_ROOM_LEAVE));
    EXPECT_TRUE(manager.isValidClientEvent(network::GameEvents::C_READY));
    EXPECT_TRUE(manager.isValidClientEvent(network::GameEvents::C_GAME_START));
}

TEST_F(NetworkManagerTest, ServerEvents_AreNotValidClientEvents) {
    EXPECT_FALSE(manager.isValidClientEvent(network::GameEvents::S_SEND_ID));
    EXPECT_FALSE(manager.isValidClientEvent(network::GameEvents::S_LOGIN_OK));
    EXPECT_FALSE(manager.isValidClientEvent(network::GameEvents::S_LOGIN_KO));
    EXPECT_FALSE(manager.isValidClientEvent(network::GameEvents::S_REGISTER_OK));
    EXPECT_FALSE(manager.isValidClientEvent(network::GameEvents::S_SNAPSHOT));
    EXPECT_FALSE(manager.isValidClientEvent(network::GameEvents::S_GAME_START));
    EXPECT_FALSE(manager.isValidClientEvent(network::GameEvents::S_ROOM_JOINED));
}

TEST_F(NetworkManagerTest, NoneEvent_IsInvalid) {
    EXPECT_FALSE(manager.isValidClientEvent(network::GameEvents::NONE));
}

TEST_F(NetworkManagerTest, TcpEvents_AreClassifiedCorrectly) {
    EXPECT_TRUE(manager.isTcpEvent(network::GameEvents::C_LOGIN));
    EXPECT_TRUE(manager.isTcpEvent(network::GameEvents::C_REGISTER));
    EXPECT_TRUE(manager.isTcpEvent(network::GameEvents::C_DISCONNECT));
    EXPECT_TRUE(manager.isTcpEvent(network::GameEvents::C_JOIN_ROOM));
    EXPECT_TRUE(manager.isTcpEvent(network::GameEvents::C_READY));
    EXPECT_TRUE(manager.isTcpEvent(network::GameEvents::C_TEAM_CHAT));
}

TEST_F(NetworkManagerTest, UdpEvents_AreClassifiedCorrectly) {
    EXPECT_TRUE(manager.isUdpEvent(network::GameEvents::C_INPUT));
    EXPECT_TRUE(manager.isUdpEvent(network::GameEvents::C_CONFIRM_UDP));
    EXPECT_TRUE(manager.isUdpEvent(network::GameEvents::C_VOICE_PACKET));
}

TEST_F(NetworkManagerTest, TcpEvents_AreNotUdp) {
    EXPECT_FALSE(manager.isUdpEvent(network::GameEvents::C_LOGIN));
    EXPECT_FALSE(manager.isUdpEvent(network::GameEvents::C_REGISTER));
}

TEST_F(NetworkManagerTest, UdpEvents_AreNotTcp) {
    EXPECT_FALSE(manager.isTcpEvent(network::GameEvents::C_INPUT));
    EXPECT_FALSE(manager.isTcpEvent(network::GameEvents::C_CONFIRM_UDP));
}

TEST_F(NetworkManagerTest, PayloadConstraints_ExistForKnownEvents) {
    EXPECT_TRUE(manager.getMinPayloadSize(network::GameEvents::C_LOGIN).has_value());
    EXPECT_TRUE(manager.getMaxPayloadSize(network::GameEvents::C_LOGIN).has_value());

    EXPECT_TRUE(manager.getMinPayloadSize(network::GameEvents::C_INPUT).has_value());
    EXPECT_TRUE(manager.getMaxPayloadSize(network::GameEvents::C_INPUT).has_value());
}

TEST_F(NetworkManagerTest, PayloadConstraints_LoginHasCorrectSize) {
    auto minSize = manager.getMinPayloadSize(network::GameEvents::C_LOGIN);
    auto maxSize = manager.getMaxPayloadSize(network::GameEvents::C_LOGIN);

    ASSERT_TRUE(minSize.has_value());
    ASSERT_TRUE(maxSize.has_value());

    EXPECT_EQ(minSize.value(), sizeof(network::connection_info));
    EXPECT_EQ(maxSize.value(), sizeof(network::connection_info));
}

TEST_F(NetworkManagerTest, PayloadConstraints_DisconnectAllowsEmpty) {
    auto minSize = manager.getMinPayloadSize(network::GameEvents::C_DISCONNECT);

    ASSERT_TRUE(minSize.has_value());
    EXPECT_EQ(minSize.value(), 0);
}

TEST_F(NetworkManagerTest, ValidPacket_PassesValidation) {
    network::connection_info info;
    std::strncpy(info.username, "testuser", sizeof(info.username));
    std::strncpy(info.password, "testpass", sizeof(info.password));

    auto msg = createValidMessage(network::GameEvents::C_LOGIN, info);
    auto result = manager.validateServerPacket(network::GameEvents::C_LOGIN, msg);

    EXPECT_TRUE(result.isValid());
    EXPECT_EQ(result.status, PackageValidation::VALID);
}

TEST_F(NetworkManagerTest, InvalidEvent_FailsValidation) {
    uint32_t dummyData = 42;
    auto msg = createValidMessage(network::GameEvents::S_LOGIN_OK, dummyData);

    auto result = manager.validateServerPacket(network::GameEvents::S_LOGIN_OK, msg);

    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(result.status, PackageValidation::INVALID_EVENT);
}

TEST_F(NetworkManagerTest, InvalidMagicValue_FailsValidation) {
    network::connection_info info;
    std::strncpy(info.username, "testuser", sizeof(info.username));
    std::strncpy(info.password, "testpass", sizeof(info.password));

    auto msg = createInvalidMagicMessage(network::GameEvents::C_LOGIN, info);
    auto result = manager.validateServerPacket(network::GameEvents::C_LOGIN, msg);

    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(result.status, PackageValidation::INVALID);
}

TEST_F(NetworkManagerTest, SizeMismatch_FailsValidation) {
    network::connection_info info;
    std::strncpy(info.username, "testuser", sizeof(info.username));
    std::strncpy(info.password, "testpass", sizeof(info.password));

    auto msg = createSizeMismatchMessage(network::GameEvents::C_LOGIN, info);
    auto result = manager.validateServerPacket(network::GameEvents::C_LOGIN, msg);

    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(result.status, PackageValidation::INVALID_PAYLOAD_SIZE);
}

TEST_F(NetworkManagerTest, PayloadTooSmall_FailsValidation) {
    uint32_t smallData = 1;
    auto msg = createValidMessage(network::GameEvents::C_LOGIN, smallData);

    auto result = manager.validateServerPacket(network::GameEvents::C_LOGIN, msg);

    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(result.status, PackageValidation::INVALID_PAYLOAD_SIZE);
    EXPECT_TRUE(result.errorMessage.find("too small") != std::string::npos);
}

TEST_F(NetworkManagerTest, PayloadTooLarge_FailsValidation) {
    struct OversizedData {
        network::connection_info info;
        char extra[1000];
    } bigData;

    auto msg = createValidMessage(network::GameEvents::C_LOGIN, bigData);
    auto result = manager.validateServerPacket(network::GameEvents::C_LOGIN, msg);

    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(result.status, PackageValidation::INVALID_PAYLOAD_SIZE);
    EXPECT_TRUE(result.errorMessage.find("too large") != std::string::npos);
}

TEST_F(NetworkManagerTest, EmptyPayload_ValidForDisconnect) {
    // C_DISCONNECT allows empty payload
    network::message<network::GameEvents> msg;
    msg.header.id = network::GameEvents::C_DISCONNECT;
    msg.header.magic_value = MAGIC_VALUE;
    msg.header.size = 0;

    auto result = manager.validateServerPacket(network::GameEvents::C_DISCONNECT, msg);

    EXPECT_TRUE(result.isValid());
}

TEST_F(NetworkManagerTest, EmptyPayload_InvalidForLogin) {
    network::message<network::GameEvents> msg;
    msg.header.id = network::GameEvents::C_LOGIN;
    msg.header.magic_value = MAGIC_VALUE;
    msg.header.size = 0;

    auto result = manager.validateServerPacket(network::GameEvents::C_LOGIN, msg);

    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(result.status, PackageValidation::INVALID_PAYLOAD_SIZE);
}

// ==================== Tests Input Event ====================

TEST_F(NetworkManagerTest, ValidInput_PassesValidation) {
    // Simulate input packet: action string + bool
    network::message<network::GameEvents> msg;
    std::string action = "move_left";
    bool pressed = true;

    msg << pressed << action;
    msg.header.id = network::GameEvents::C_INPUT;
    msg.header.magic_value = MAGIC_VALUE;
    msg.header.size = msg.body.size();

    auto result = manager.validateServerPacket(network::GameEvents::C_INPUT, msg);

    EXPECT_TRUE(result.isValid());
}

// ==================== Tests JoinRoom Event ====================

TEST_F(NetworkManagerTest, ValidJoinRoom_PassesValidation) {
    uint32_t lobbyId = 12345;
    auto msg = createValidMessage(network::GameEvents::C_JOIN_ROOM, lobbyId);

    auto result = manager.validateServerPacket(network::GameEvents::C_JOIN_ROOM, msg);

    EXPECT_TRUE(result.isValid());
}

// ==================== Edge Cases ====================

TEST_F(NetworkManagerTest, ValidationResult_ErrorMessageIsEmptyOnSuccess) {
    uint32_t lobbyId = 1;
    auto msg = createValidMessage(network::GameEvents::C_JOIN_ROOM, lobbyId);

    auto result = manager.validateServerPacket(network::GameEvents::C_JOIN_ROOM, msg);

    EXPECT_TRUE(result.isValid());
    EXPECT_TRUE(result.errorMessage.empty());
}

TEST_F(NetworkManagerTest, ValidationResult_ErrorMessageIsSetOnFailure) {
    uint32_t dummyData = 42;
    auto msg = createValidMessage(network::GameEvents::S_LOGIN_OK, dummyData);

    auto result = manager.validateServerPacket(network::GameEvents::S_LOGIN_OK, msg);

    EXPECT_FALSE(result.isValid());
    EXPECT_FALSE(result.errorMessage.empty());
}
