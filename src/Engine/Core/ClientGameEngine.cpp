#include "ClientGameEngine.hpp"
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <ostream>

#include "Components/NetworkComponents.hpp"
#include "Components/StandardComponents.hpp"
#include "Components/serialize/StandardComponents_serialize.hpp"
#include "Components/serialize/score_component_serialize.hpp"
#include "Network.hpp"
#include "NetworkEngine/NetworkEngine.hpp"
#include "AudioSystem.hpp"

#include "../../../RType/Common/Systems/health.hpp"
#include "../../../RType/Common/Systems/score.hpp"
#include "../../../RType/Common/Components/spawn.hpp"
#include "../../../RType/Common/Components/shooter_component.hpp"
#include "../../../RType/Common/Components/charged_shot.hpp"
#include "../../../RType/Common/Components/team_component.hpp"
#include "../../../RType/Common/Components/damage_component.hpp"
#include "../../../RType/Common/Components/game_timer.hpp"
#include "../../../RType/Common/Components/pod_component.hpp"
#include "../../../RType/Common/Components/game_over_notification.hpp"
#include "../../../RType/Common/Systems/ai_behavior.hpp"
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
    registerNetworkComponent<::GameTimerComponent>();
    registerNetworkComponent<AudioSourceComponent>();

    // R-Type specific components
    registerNetworkComponent<PodComponent>();
    registerNetworkComponent<PlayerPodComponent>();
    registerNetworkComponent<AIBehaviorComponent>();
    registerNetworkComponent<BossComponent>();
    registerNetworkComponent<ScoreComponent>();

    _ecs.systems.addSystem<BackgroundSystem>();
    _ecs.systems.addSystem<RenderSystem>();
    _ecs.systems.addSystem<AudioSystem>();
    //_ecs.systems.addSystem<InputSystem>(input_manager);

    // Physics and game logic handled by server - client only renders
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

    if (pending.count(network::GameEvents::S_ENTITY_DESTROY)) {
        auto& destroy_packets = pending.at(network::GameEvents::S_ENTITY_DESTROY);
        for (const auto& msg : destroy_packets) {
            auto mutable_msg = msg;
            uint32_t guid;
            mutable_msg >> guid;

            auto it = _networkToLocalEntity.find(guid);
            if (it != _networkToLocalEntity.end()) {
                Entity localId = it->second;
                _ecs.registry.destroyEntity(localId);
                _networkToLocalEntity.erase(it);
                std::cout << "[CLIENT] Destroyed entity guid=" << guid << " localId=" << localId << std::endl;
            }
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
        }
    }

    // Nouveau: Gérer le message S_GAME_OVER
    if (pending.count(network::GameEvents::S_GAME_OVER)) {
        auto& msgs = pending.at(network::GameEvents::S_GAME_OVER);
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
        std::cout << "🎮 [CLIENT] ========= S_GAME_OVER PACKET RECEIVED =========" << std::endl;
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
        for (auto& msg : msgs) {
            network::GameOverPacket packet;
            msg >> packet;
            
            std::cout << "🎮 [CLIENT] Victory: " << (packet.victory ? "YES" : "NO") << std::endl;
            std::cout << "🎮 [CLIENT] Total players in packet: " << (int)packet.player_count << std::endl;
            
            // Créer un composant pour signaler le game over au GameManager
            Entity gameOverEntity = _ecs.registry.createEntity();
            GameOverNotification notification;
            notification.victory = packet.victory;
            _ecs.registry.addComponent<GameOverNotification>(gameOverEntity, notification);
            
            std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
            std::cout << "🎮 [CLIENT] Creating " << (int)packet.player_count << " LEADERBOARD_DATA entities..." << std::endl;
            std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
            
            // Créer des entités temporaires pour tous les joueurs avec leurs scores
            // Le GameManager pourra les lire pour afficher le leaderboard complet
            for (uint32_t i = 0; i < packet.player_count; i++) {
                Entity playerScoreEntity = _ecs.registry.createEntity();
                
                std::cout << "  🔵 Creating entity " << playerScoreEntity << " for player " << packet.players[i].client_id << std::endl;
                
                // Tag comme joueur pour le leaderboard
                TagComponent tags;
                tags.tags.push_back("PLAYER");
                tags.tags.push_back("LEADERBOARD_DATA");
                _ecs.registry.addComponent<TagComponent>(playerScoreEntity, tags);
                std::cout << "    ✅ Added tags: PLAYER, LEADERBOARD_DATA" << std::endl;
                
                // Score du joueur
                ScoreComponent score;
                score.current_score = packet.players[i].score;
                score.high_score = 0;
                _ecs.registry.addComponent<ScoreComponent>(playerScoreEntity, score);
                std::cout << "    ✅ Added ScoreComponent: " << packet.players[i].score << " pts" << std::endl;
                
                // État vivant/mort
                HealthComponent health;
                health.current_hp = packet.players[i].is_alive ? 1 : 0;
                health.max_hp = 1;
                health.last_damage_time = 0;
                _ecs.registry.addComponent<HealthComponent>(playerScoreEntity, health);
                std::cout << "    ✅ Added HealthComponent: HP=" << health.current_hp << std::endl;
                
                // IMPORTANT: Stocker le client_id dans NetworkIdentity pour l'affichage
                NetworkIdentity net_id;
                net_id.guid = packet.players[i].client_id;
                net_id.ownerId = packet.players[i].client_id;
                _ecs.registry.addComponent<NetworkIdentity>(playerScoreEntity, net_id);
                std::cout << "    ✅ Added NetworkIdentity: ownerId=" << net_id.ownerId << std::endl;
                
                std::cout << "  ✅ [CLIENT] Created LEADERBOARD entity " << playerScoreEntity 
                          << " for Player " << packet.players[i].client_id 
                          << " (score=" << packet.players[i].score 
                          << ", alive=" << packet.players[i].is_alive << ")" << std::endl;
            }
            
            std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
            std::cout << "🎮 [CLIENT] ✅ ALL " << (int)packet.player_count << " LEADERBOARD ENTITIES CREATED" << std::endl;
            std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
        }
    }
}

int ClientGameEngine::run() {
    system_context context = {0,
                              _currentTick,
                              _texture_manager,
                              _sound_manager,
                              _music_manager,
                              _window_manager.getWindow(),
                              input_manager,
                              _clientId};
    auto last_time = std::chrono::high_resolution_clock::now();

    // Environment mode: CLIENT for multiplayer (server handles all game logic)
    Environment env(_ecs, _texture_manager, _sound_manager, _music_manager, EnvMode::CLIENT);

    this->init();
    if (_init_function)
        _init_function(env, input_manager);

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
            _loop_function(env, input_manager);
        _ecs.update(context);

        _window_manager.display();
        _currentTick++;
    }
    return SUCCESS;
}
