#include <string>
#include "GameManager.hpp"
#include <iostream>
#include <memory>
#include <utility>
#include "ECS.hpp"
#include "GameEngineBase.hpp"
#include "src/RType/Common/Systems/health.hpp"
#include "src/RType/Common/Systems/shooter.hpp"
#include "src/RType/Common/Systems/damage.hpp"
#include "src/RType/Common/Systems/spawn.hpp"
#include "src/RType/Common/Systems/wall_collision.hpp"
#include "src/RType/Common/Components/charged_shot.hpp"
#include "src/RType/Common/Components/status_display_components.hpp"
#include "src/RType/Common/Components/terrain_component.hpp"
#include "src/RType/Common/Systems/ai_behavior.hpp"
#include "src/RType/Common/Systems/score.hpp"
#include "src/RType/Common/Systems/animation_helper.hpp"
#include "src/RType/Common/Systems/powerup.hpp"
#include "src/RType/Common/Systems/boss_patterns.hpp"
#include "src/RType/Common/Systems/boss_tail.hpp"
#include "src/RType/Common/Systems/projectile_cleanup.hpp"
#include "src/RType/Common/Systems/pod_system.hpp"
#include "src/RType/Common/Systems/status_display.hpp"
#include "src/RType/Common/Systems/level_transition.hpp"
#include "src/RType/Common/Systems/game_state_system.hpp"
#include "src/Engine/Lib/Systems/PatternSystem/PatternSystem.hpp"
#include "src/Engine/Lib/Systems/PlayerBoundsSystem.hpp"
#include "src/Engine/Core/Scene/SceneLoader.hpp"
#include "src/RType/Common/Scene/ScenePrefabs.hpp"
#include "CollisionSystem.hpp"

#include "src/Engine/Lib/Systems/PhysicsSystem.hpp"
#include "src/Engine/Lib/Systems/ActionScriptSystem.hpp"
#include "src/Engine/Lib/Systems/DestructionSystem.hpp"

void GameManager::initSystems(Environment& env) {
    auto& ecs = env.getECS();

    // Game logic systems - server only (NOT client)
    // Le serveur gère toute la logique du jeu
    // Le client reçoit juste les snapshots et affiche
    if (!env.isClient()) {
        std::cout << "[GameManager] Initializing gameplay systems (Server mode)" << std::endl;
        ecs.systems.addSystem<BoxCollision>();  // Collision detection must run before damage
        ecs.systems.addSystem<ShooterSystem>();
        ecs.systems.addSystem<ProjectileCleanupSystem>();  // Nouveau : Nettoie projectiles hors écran
        ecs.systems.addSystem<PowerUpSystem>();  // Nouveau : Power-Ups après collisions
        ecs.systems.addSystem<Damage>();
        ecs.systems.addSystem<HealthSystem>();
        ecs.systems.addSystem<PatternSystem>();
        ecs.systems.addSystem<EnemySpawnSystem>();
        ecs.systems.addSystem<WallCollisionSystem>();
        ecs.systems.addSystem<PodSystem>();
        ecs.systems.addSystem<AIBehaviorSystem>();
        ecs.systems.addSystem<BossPatternSystem>();  // Nouveau : Patterns complexes du boss
        ecs.systems.addSystem<BossTailSystem>();  // Nouveau : Animation de la queue du boss
        ecs.systems.addSystem<BoundsSystem>();
        ecs.systems.addSystem<PlayerBoundsSystem>();
        ecs.systems.addSystem<ScoreSystem>();
        ecs.systems.addSystem<GameStateSystem>();  // Nouveau : Détecte game over et envoie messages réseau
        ecs.systems.addSystem<PhysicsSystem>();
        ecs.systems.addSystem<ActionScriptSystem>();
    } else {
        std::cout << "[GameManager] Skipping gameplay systems (Client mode - server handles all logic)" << std::endl;
    }

    // Destruction system runs on both server (to send packets) and client (to clean up)
    // Actually, on server it sends packets AND cleans up.
    // On client (standalone), it cleans up.
    // In multiplayer client mode, received S_ENTITY_DESTROY handles destruction, BUT
    // local entities (predictive inputs etc) might use PendingDestruction too.
    ecs.systems.addSystem<DestructionSystem>();

    // Client-only systems (rendering, UI)
    if (!env.isServer()) {
        ecs.systems.addSystem<StatusDisplaySystem>();
        ecs.systems.addSystem<LevelTransitionSystem>();
    }
}

void GameManager::initBackground(Environment& env, const LevelConfig& config) {
    if (!env.isServer()) {
        std::string bgPath = config.background_texture;
        if (bgPath.empty()) {
            bgPath = "src/RType/Common/content/sprites/test.png";
        }

        auto& ecs = env.getECS();
        Entity bgEntity = ecs.registry.createEntity();

        BackgroundComponent bg{};
        bg.x_offset = 0.f;
        bg.scroll_speed = 60.f;

        bg.texture_handle = env.loadTexture(bgPath);

        ecs.registry.addComponent<BackgroundComponent>(bgEntity, bg);
    }
}

void GameManager::initBounds(Environment& env) {
    auto& ecs = env.getECS();

    _boundsEntity = ecs.registry.createEntity();
    WorldBoundsComponent bounds;
    bounds.min_x = 0.0f;
    bounds.min_y = 0.0f;

    ecs.registry.addComponent<WorldBoundsComponent>(_boundsEntity, bounds);
}

void GameManager::initPlayer(Environment& env) {
    // In multiplayer mode (server or client), players are spawned when clients connect
    // and replicated via network snapshots. Only spawn locally in standalone mode.
    if (env.isServer() || env.isClient()) {
        std::cout << "[GameManager] Multiplayer mode: Player spawning handled by network replication" << std::endl;
        return;
    }

    auto& ecs = env.getECS();

    float start_x = _player_config.start_x.value();
    float start_y = _player_config.start_y.value();

    _player = env.spawn<Player>(SpawnPolicy::PREDICTED, std::pair<float, float>{start_x, start_y});
    if (_player) {
        _player->setTexture(_player_config.sprite_path.value());

        // Configuration de l'animation horizontale avec le helper
        Entity player_id = _player->getId();
        int num_frames = _player_config.animation_frames.value_or(5);
        float anim_speed = _player_config.animation_speed.value_or(0.1f);
        AnimationHelper::setupHorizontalAnimation(ecs.registry, player_id, _player_config, num_frames, anim_speed);

        // Appliquer le scale si defini dans la configuration
        if (_player_config.scale.has_value()) {
            std::cout << "[GameManager] Applying player scale: " << _player_config.scale.value() << std::endl;
            _player->setScale({_player_config.scale.value(), _player_config.scale.value()});
        } else {
            std::cout << "[GameManager] No scale defined in player config" << std::endl;
        }

        _player->setFireRate(_player_config.fire_rate.value());
        _player->setLifePoint(_player_config.hp.value());
        _player->setLifePoint(_player_config.hp.value());

        _player->addCollisionTag("AI");
        _player->addCollisionTag("ENEMY_PROJECTILE");
        _player->addCollisionTag("OBSTACLE");
        _player->addCollisionTag("ITEM");
        _player->addCollisionTag("POWERUP");  // Nouveau : pour collecter les power-ups
        _player->addCollisionTag("WALL");

        ChargedShotComponent charged_shot;
        charged_shot.min_charge_time = 0.5f;
        charged_shot.max_charge_time = 2.0f;
        ecs.registry.addComponent<ChargedShotComponent>(_player->getId(), charged_shot);

        PlayerPodComponent player_pod;
        player_pod.has_pod = false;
        player_pod.pod_entity = -1;
        player_pod.pod_attached = false;
        player_pod.last_known_hp = _player_config.hp.value();
        ecs.registry.addComponent<PlayerPodComponent>(_player->getId(), player_pod);
    }
}

void GameManager::initSpawner(Environment& env, const LevelConfig& config) {
    auto& ecs = env.getECS();

    if (!env.isClient()) {
        // Créer le timer de jeu avec NetworkIdentity pour la réplication
        Entity timer_entity = ecs.registry.createEntity();
        ecs.registry.addComponent<GameTimerComponent>(timer_entity, {0.0f});
        NetworkIdentity timer_net_id;
        timer_net_id.guid = timer_entity;
        timer_net_id.ownerId = 0;  // Server-owned
        ecs.registry.addComponent<NetworkIdentity>(timer_entity, timer_net_id);

        Entity spawner = ecs.registry.createEntity();
        EnemySpawnComponent spawn_comp;
        spawn_comp.spawn_interval = 2.0f;
        spawn_comp.is_active = true;
        
        // Use configuration paths from LevelConfig
        spawn_comp.enemies_config_path = config.enemies_config;
        spawn_comp.boss_config_path = config.boss_config;
        spawn_comp.game_config_path = config.game_config;

        ecs.registry.addComponent<EnemySpawnComponent>(spawner, spawn_comp);
        ecs.registry.addComponent<NetworkIdentity>(spawner, {static_cast<uint32_t>(spawner), 0});

        // Scripted Spawn Component
        Entity scripted_spawner = ecs.registry.createEntity();
        ScriptedSpawnComponent scripted_spawn_comp;
        scripted_spawn_comp.script_path = config.spawn_script;
        ecs.registry.addComponent<ScriptedSpawnComponent>(scripted_spawner, scripted_spawn_comp);

        Entity pod_spawner = ecs.registry.createEntity();
        PodSpawnComponent pod_spawn_comp;
        pod_spawn_comp.spawn_interval = 15.0f;
        pod_spawn_comp.min_spawn_interval = 10.0f;
        pod_spawn_comp.max_spawn_interval = 20.0f;
        pod_spawn_comp.can_spawn = true;
        ecs.registry.addComponent<PodSpawnComponent>(pod_spawner, pod_spawn_comp);
    }
}

void GameManager::initUI(Environment& env) {
    auto& ecs = env.getECS();

    if (!env.isServer()) {
        // Timer UI (top left)
        _timerEntity = ecs.registry.createEntity();
        ecs.registry.addComponent<TextComponent>(
            _timerEntity, {"Time: 0s", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 28,
                           sf::Color::Cyan, 10, 10});

        // Boss HP UI (haut à droite)
        _bossHPEntity = ecs.registry.createEntity();
        ecs.registry.addComponent<TextComponent>(
            _bossHPEntity, {"", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 28,
                           sf::Color::Red, 1400, 10});

        // Score tracker
        _scoreTrackerEntity = ecs.registry.createEntity();
        ecs.registry.addComponent<ScoreComponent>(_scoreTrackerEntity, {0, 0});

        // Status Display Component (links player to UI)
        _statusDisplayEntity = ecs.registry.createEntity();
        StatusDisplayComponent statusDisplay;
        statusDisplay.is_initialized = true;
        ecs.registry.addComponent<StatusDisplayComponent>(_statusDisplayEntity, statusDisplay);

        // Charge Bar (bottom center)
        _chargeBarEntity = ecs.registry.createEntity();
        ChargeBarComponent chargeBar;
        chargeBar.bar_width = 200.0f;
        chargeBar.bar_height = 20.0f;
        chargeBar.x = 860.0f;
        chargeBar.y = 1030.0f;
        ecs.registry.addComponent<ChargeBarComponent>(_chargeBarEntity, chargeBar);

        // Lives Display (bottom left)
        _livesEntity = ecs.registry.createEntity();
        LivesDisplayComponent livesDisplay;
        livesDisplay.x = 50.0f;
        livesDisplay.y = 1030.0f;
        livesDisplay.icon_size = 32.0f;
        livesDisplay.icon_spacing = 40.0f;
        ecs.registry.addComponent<LivesDisplayComponent>(_livesEntity, livesDisplay);

        // Score Display (bottom right) - R-Type style with 7 zeros
        _scoreDisplayEntity = ecs.registry.createEntity();
        ScoreDisplayComponent scoreDisplay;
        scoreDisplay.digit_count = 7;
        scoreDisplay.x = 1650.0f;
        scoreDisplay.y = 1030.0f;
        ecs.registry.addComponent<ScoreDisplayComponent>(_scoreDisplayEntity, scoreDisplay);
    }
}

void GameManager::initScene(Environment& env, const LevelConfig& config) {
    // Only server loads the scene - clients receive entities via network replication
    if (env.isClient()) {
        std::cout << "GameManager: Client mode - scene will be received from server" << std::endl;
        return;
    }

    auto& ecs = env.getECS();

    _scene_manager = std::make_unique<SceneManager>(ecs.registry);

    ScenePrefabs::registerAll(*_scene_manager, env.getTextureManager());

    try {
        _scene_manager->loadScene(config);
        std::cout << "GameManager: Loaded scene '" << config.name << "'" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "GameManager: Failed to load scene: " << e.what() << std::endl;
    }
}
