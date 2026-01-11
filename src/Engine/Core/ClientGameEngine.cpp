#include "ClientGameEngine.hpp"
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <ostream>

#include "Components/NetworkComponents.hpp"
#include "Components/StandardComponents.hpp"
#include "Network.hpp"
#include "NetworkEngine/NetworkEngine.hpp"

#include "../../../RType/Common/Components/health.hpp"
#include "../../../RType/Common/Components/spawn.hpp"
#include "../../../RType/Common/Components/shooter_component.hpp"
#include "../../../RType/Common/Components/charged_shot.hpp"
#include "../../../RType/Common/Components/team_component.hpp"
#include "../../../RType/Common/Components/damage_component.hpp"
#include "Components/StandardComponents.hpp"
#include "Components/NetworkComponents.hpp"

ClientGameEngine::ClientGameEngine(std::string window_name) : _window_manager(WINDOW_W, WINDOW_H, window_name) {
    _network = std::make_unique<engine::core::NetworkEngine>(engine::core::NetworkEngine::NetworkRole::CLIENT);
}

int ClientGameEngine::init() {
    _network->transmitEvent<int>(network::GameEvents::C_LOGIN_ANONYMOUS, 0, 0, 0);

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
    registerNetworkComponent<PatternComponent>();
    registerNetworkComponent<ProjectileComponent>();
    registerNetworkComponent<TeamComponent>();
    registerNetworkComponent<DamageOnCollision>();
    registerNetworkComponent<NetworkIdentity>();

    _ecs.systems.addSystem<BackgroundSystem>();
    _ecs.systems.addSystem<RenderSystem>();
    //_ecs.systems.addSystem<InputSystem>(input_manager);

    // if mode local or prediction (?)
    _ecs.systems.addSystem<PhysicsSystem>();
    // _ecs.systems.addSystem<BoxCollision>();
    // _ecs.systems.addSystem<ActionScriptSystem>();
    // _ecs.systems.addSystem<PatternSystem>();
    // _ecs.systems.addSystem<SpawnSystem>();
    return 0;
}

void ClientGameEngine::handleEvent() {
    while (std::optional<sf::Event> event = _window_manager.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            _window_manager.getWindow().close();
        if (event->is<sf::Event::FocusLost>())
            input_manager.setWindowHasFocus(false);
        if (event->is<sf::Event::FocusGained>())
            input_manager.setWindowHasFocus(true);
    }
}

void ClientGameEngine::processNetworkEvents() {
    _network->processIncomingPackets(_currentTick);
    auto pending = _network->getPendingEvents();

    if (pending.count(network::GameEvents::S_SNAPSHOT)) {
        auto& snapshot_packets = pending.at(network::GameEvents::S_SNAPSHOT);

        for (const auto& msg : snapshot_packets) {
            auto mutable_msg = msg;
            ComponentPacket packet;
            mutable_msg >> packet;
            processComponentPacket(packet.entity_guid, packet.component_type, packet.data);
        }
    }

    if (pending.count(network::GameEvents::S_VOICE_RELAY)) {
        std::cout << "Vocal replay not implemented" << std::endl;
    }

    if (pending.count(network::GameEvents::S_ASSIGN_PLAYER_ENTITY)) {
        auto& msgs = pending.at(network::GameEvents::S_ASSIGN_PLAYER_ENTITY);
        for (auto& msg : msgs) {
            network::AssignPlayerEntityPacket packet;
            msg >> packet;
            _localPlayerEntity = packet.entityId;
            std::cout << "CLIENT: Assigned player entity " << packet.entityId << std::endl;
        }
    }
}

int ClientGameEngine::run() {
    system_context context = {0, _currentTick, _texture_manager, _window_manager.getWindow(), input_manager, _clientId};
    auto last_time = std::chrono::high_resolution_clock::now();

    this->init();
    if (_init_function)
        _init_function(*this, input_manager);

    while (_window_manager.isOpen()) {
        auto now = std::chrono::high_resolution_clock::now();
        context.dt = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count() / 1000.0f;
        last_time = now;

        if (context.player_id == 0)
            context.player_id = _clientId;

        handleEvent();
        processNetworkEvents();
        input_manager.update(*_network, _currentTick, context);
        _window_manager.clear();

        if (_loop_function)
            _loop_function(*this, input_manager);
        _ecs.update(context);

        _window_manager.display();
        _currentTick++;
    }
    return SUCCESS;
}
