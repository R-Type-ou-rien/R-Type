#include "ServerGameEngine.hpp"
#include <chrono>
#include "Components/NetworkComponents.hpp"
#include "Context.hpp"
#include "GameEngineBase.hpp"
#include "Network.hpp"
#include "NetworkEngine/NetworkEngine.hpp"

ServerGameEngine::ServerGameEngine() {
    _network = std::make_unique<engine::core::NetworkEngine>(engine::core::NetworkEngine::NetworkRole::SERVER);
}

int ServerGameEngine::init() {
    _ecs.systems.addSystem<BoxCollision>();
    _ecs.systems.addSystem<SpawnSystem>();
    _ecs.systems.addSystem<ActionScriptSystem>();
    _ecs.systems.addSystem<PatternSystem>();
    _ecs.systems.addSystem<PhysicsSystem>();
    
    _ecs.systems.addSystem<ComponentSenderSystem>();

    return SUCCESS;
}

void ServerGameEngine::processNetworkEvents()
{
    _network->processIncomingPackets(_currentTick);
    auto pending = _network->getPendingEvents(); 

    if (pending.count(network::GameEvents::C_INPUT)) {
        ActionPacket packet;
        for (auto data : pending[network::GameEvents::C_INPUT]) {
            std::memcpy(&packet, data.data(), sizeof(ActionPacket));
            input_manager.updateActionFromPacket(packet);
        }
    }
}

int ServerGameEngine::run() {
    system_context ctx = {0, _currentTick, _texture_manager, input_manager, *_network};
    auto last_time = std::chrono::high_resolution_clock::now();
    Environment env(_ecs, _texture_manager, *_network, EnvMode::SERVER);

    init();
    if (_init_function)
        _init_function(env, input_manager);

    while (1) {
        auto now = std::chrono::high_resolution_clock::now();
        ctx.dt = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count() / 1000.0f;
        last_time = now;

        processNetworkEvents();

        if (_loop_function)
            _loop_function(env, input_manager);
        _ecs.update(ctx);

        _currentTick++;
    }
    return SUCCESS;
}