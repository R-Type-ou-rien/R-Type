#include "ClientGameEngine.hpp"
#include <chrono>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <iostream>
#include <ostream>
#include "CollisionSystem.hpp"
#include "ActionScriptSystem.hpp"
#include "Components/NetworkComponents.hpp"
#include "Components/StandardComponents.hpp"
#include "GameEngineBase.hpp"
#include "Hash/Hash.hpp"
#include "NetworkEngine/NetworkEngine.hpp"
#include "PatternSystem/PatternSystem.hpp"
#include "SpawnSystem.hpp"

ClientGameEngine::ClientGameEngine(std::string window_name)
: _window_manager(WINDOW_W, WINDOW_H, window_name)
{
    _network = std::make_unique<engine::core::NetworkEngine>(engine::core::NetworkEngine::NetworkRole::CLIENT);
}

int ClientGameEngine::init() {
    registerNetworkComponent<transform_component_s>();
    registerNetworkComponent<Velocity2D>();
    registerNetworkComponent<sprite2D_component_s>();
    registerNetworkComponent<BackgroundComponent>();
    registerNetworkComponent<ResourceComponent>();
    registerNetworkComponent<TextComponent>();

    _ecs.systems.addSystem<BackgroundSystem>();
    _ecs.systems.addSystem<RenderSystem>();
    //_ecs.systems.addSystem<InputSystem>(input_manager);

    // if mode local or prediction (?)
    _ecs.systems.addSystem<PhysicsSystem>();
    _ecs.systems.addSystem<BoxCollision>();
    _ecs.systems.addSystem<ActionScriptSystem>();
    _ecs.systems.addSystem<PatternSystem>();
    _ecs.systems.addSystem<SpawnSystem>();
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
        ComponentPacket packet;
        for (auto data : pending[network::GameEvents::S_SNAPSHOT]) {
            std::memcpy(&packet, data.data(), sizeof(ComponentPacket));
            processComponentPacket(packet.entity_guid, packet.component_type, packet.data);
        }
    }

    if (pending.count(network::GameEvents::S_VOICE_RELAY)) {
        std::cout << "Vocal replay not implemented" << std::endl; 
    }
}


int ClientGameEngine::run() {
    system_context context = {0, _currentTick, _texture_manager, _window_manager.getWindow(), input_manager};
    auto last_time = std::chrono::high_resolution_clock::now();
    Environment env(_ecs, _texture_manager, *_network, EnvMode::STANDALONE);

    this->init();
    if (_init_function)
        _init_function(env, input_manager);

    while (_window_manager.isOpen()) {
        auto now = std::chrono::high_resolution_clock::now();
        context.dt = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count() / 1000.0f;
        last_time = now;

        handleEvent();
        processNetworkEvents();
        input_manager.update(*_network, _currentTick, context.dt);
        _window_manager.clear();

        if (_loop_function)
            _loop_function(env, input_manager);
        _ecs.update(context);

        _window_manager.display();
        _currentTick++;
    }
    return SUCCESS;
}
