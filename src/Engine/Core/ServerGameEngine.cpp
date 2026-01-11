#include "ServerGameEngine.hpp"
#include <chrono>
#include <iostream>
#include <ostream>
#include <thread>
#include "Components/NetworkComponents.hpp"
#include "Context.hpp"
#include "GameEngineBase.hpp"
#include "Network.hpp"
#include "NetworkEngine/NetworkEngine.hpp"
#include "Components/StandardComponents.hpp"
#include "../../RType/Common/Components/health.hpp"
#include "../../RType/Common/Components/spawn.hpp"
#include "../../RType/Common/Components/shooter.hpp"
#include "../../RType/Common/Components/charged_shot.hpp"

ServerGameEngine::ServerGameEngine() {
    _network = std::make_unique<engine::core::NetworkEngine>(engine::core::NetworkEngine::NetworkRole::SERVER);
    // Create a default lobby that players can join
    _lobbyManager.createLobby("Default Lobby", 4);
}

int ServerGameEngine::init() {
    // Systems are added in GameManager::initSystems()
    // Only add server-specific systems here
    _ecs.systems.addSystem<ComponentSenderSystem>();

    registerNetworkComponent<sprite2D_component_s>();
    registerNetworkComponent<transform_component_s>();
    registerNetworkComponent<Velocity2D>();
    registerNetworkComponent<BoxCollisionComponent>();
    registerNetworkComponent<TagComponent>();
    registerNetworkComponent<HealthComponent>();
    registerNetworkComponent<EnemySpawnComponent>();
    registerNetworkComponent<ShooterComponent>();
    registerNetworkComponent<ChargedShotComponent>();
    registerNetworkComponent<TextComponent>();
    registerNetworkComponent<ResourceComponent>();
    registerNetworkComponent<BackgroundComponent>();

    return SUCCESS;
}

void ServerGameEngine::processNetworkEvents() {
    _network->processIncomingPackets(_currentTick);
    auto pending = _network->getPendingEvents();

    // Handle new connections
    if (pending.count(network::GameEvents::C_CONNECTION)) {
        for (const auto& msg : pending.at(network::GameEvents::C_CONNECTION)) {
            uint32_t newClientId = msg.header.user_id;
            _lobbyManager.onClientConnected(newClientId);
            // Automatically join the default lobby for now. A real game would have lobby selection.
            _lobbyManager.joinLobby(1, newClientId);
            std::cout << "SERVER: Client " << newClientId << " connected and joined lobby 1." << std::endl;
        }
    }

    // Handle disconnections
    if (pending.count(network::GameEvents::C_DISCONNECT)) {
        for (const auto& msg : pending.at(network::GameEvents::C_DISCONNECT)) {
            uint32_t clientId = msg.header.user_id;
            _lobbyManager.onClientDisconnected(clientId);
            _clientToEntityMap.erase(clientId);  // Clean up entity mapping
            std::cout << "SERVER: Client " << clientId << " disconnected." << std::endl;
        }
    }

    // Handle inputs
    if (pending.count(network::GameEvents::C_INPUT)) {
        auto& input_messages = pending.at(network::GameEvents::C_INPUT);
        for (auto& msg : input_messages) {
            ActionPacket packet;
            msg >> packet;
            updateActions(packet, msg.header.user_id);
        }
    }
}

void ServerGameEngine::updateActions(ActionPacket& packet, uint32_t clientId) {
    auto it = _clientToEntityMap.find(clientId);

    input_manager.updateActionFromPacket(packet, clientId);
}

int ServerGameEngine::run() {
    system_context ctx = {0, _currentTick, _texture_manager, input_manager, *_network, {}, &_lobbyManager};
    auto last_time = std::chrono::high_resolution_clock::now();

    init();

    if (_init_function)
        _init_function(*this, input_manager);

    while (1) {
        auto now = std::chrono::high_resolution_clock::now();
        ctx.dt = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count() / 1000.0f;
        last_time = now;

        processNetworkEvents();

        if (_loop_function) {
            _loop_function(*this, input_manager);
        }

        // Populate active clients for systems
        ctx.active_clients.clear();
        for (const auto& [lobbyId, lobby] : _lobbyManager.getAllLobbies()) {
            if (lobby.getState() == engine::core::Lobby::State::IN_GAME) {
                for (const auto& client : lobby.getClients()) {
                    ctx.active_clients.push_back(client.id);
                }
            }
        }

        _ecs.update(ctx);

        // Reset one-frame input flags (justPressed, justReleased) after processing
        input_manager.resetFrameFlags();

        _currentTick++;
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    return SUCCESS;
}