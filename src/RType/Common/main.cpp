#include <iostream>
#include <ostream>

#include "GameEngineConfig.hpp"
#include "NetworkEngine/NetworkEngine.hpp"
#include "Components/StandardComponents.hpp"
#include "Lib/GameManager/GameManager.hpp"

int main() {
    GameEngine engine;
    GameManager gm;

#if defined(CLIENT_BUILD)
    gm.setWindow(&engine.getWindow());
    gm.setLocalPlayerId(engine.getClientId());

    // Connect network callbacks
    gm.setRequestLobbyListCallback([&engine]() { engine.requestLobbyList(); });
    gm.setCreateLobbyCallback([&engine](const std::string& name) { engine.createLobby(name); });
    gm.setJoinLobbyCallback([&engine](uint32_t lobbyId) { engine.joinLobby(lobbyId); });
    gm.setLeaveLobbyCallback([&engine](uint32_t lobbyId) { engine.sendLeaveLobby(lobbyId); });
    gm.setStartGameCallback([&engine](uint32_t lobbyId) { engine.sendStartGame(); });
    gm.setGetAvailableLobbiesCallback([&engine]() -> std::vector<LobbyInfo> {
        auto& engineLobbies = engine.getAvailableLobbies();
        std::vector<LobbyInfo> result;
        for (const auto& l : engineLobbies) {
            result.push_back({l.id, l.name, static_cast<int>(l.playerCount), static_cast<int>(l.maxPlayers)});
        }
        return result;
    });

    gm.setReadyCallback([&engine](bool ready) {
        if (ready)
            engine.sendReady();
        else
            engine.sendUnready();
    });

    // Connect Auth callbacks
    gm.setLoginCallback([&engine](const std::string& user, const std::string& pass) { engine.sendLogin(user, pass); });
    gm.setRegisterCallback(
        [&engine](const std::string& user, const std::string& pass) { engine.sendRegister(user, pass); });
    gm.setAnonymousLoginCallback([&engine]() { engine.sendAnonymousLogin(); });

    engine.setAuthSuccessCallback([&gm]() { gm.onAuthSuccess(); });
    engine.setAuthFailedCallback([&gm]() { gm.onAuthFailed(); });
    engine.setPlayerJoinedCallback([&gm](const engine::core::LobbyPlayerInfo& p) {
        LobbyPlayerInfo info;
        info.id = p.id;
        info.name = p.name;
        info.isReady = p.isReady;
        info.isHost = p.isHost;
        gm.onPlayerJoined(info);
    });

    engine.setGameStartedCallback([&gm]() { gm.onGameStarted(); });

    engine.setPlayerLeftCallback([&gm](uint32_t id) { gm.onPlayerLeft(id); });

    engine.setNewHostCallback([&gm](uint32_t id) { gm.onNewHost(id); });

    engine.setReadyChangedCallback([&gm](uint32_t id, bool ready) { gm.onPlayerReadyChanged(id, ready); });

    engine.setChatMessageCallback(
        [&gm](const std::string& sender, const std::string& msg) { gm.onChatMessageReceived(sender, msg); });

    gm.setSendChatCallback([&engine](const std::string& msg) { engine.sendChatMessage(msg); });

    engine.setFocusChangedCallback([&gm](bool hasFocus) { gm.setWindowFocus(hasFocus); });

    engine.setLobbyJoinedCallback([&gm](uint32_t id, const std::string& name,
                                        const std::vector<engine::core::LobbyPlayerInfo>& players, uint32_t hostId) {
        std::vector<LobbyPlayerInfo> gmPlayers;
        for (const auto& p : players) {
            LobbyPlayerInfo info;
            info.id = p.id;
            info.name = p.name;
            info.isReady = p.isReady;
            info.isHost = p.isHost;
            gmPlayers.push_back(info);
        }
        gm.onLobbyJoined(id, name, gmPlayers, hostId);
    });
#endif

    engine.setInitFunction([&gm](Environment& env, InputManager& inputs) { gm.init(env, inputs); });

    engine.setLoopFunction([&gm](Environment& env, InputManager& inputs) { gm.update(env, inputs); });
    engine.run();
    return 0;
}
