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
        ecs.systems.addSystem<Damage>();
        ecs.systems.addSystem<HealthSystem>();
        ecs.systems.addSystem<ProjectileCleanupSystem>();

        // IA et Patterns
        ecs.systems.addSystem<AIBehaviorSystem>();
        ecs.systems.addSystem<PatternSystem>();
        ecs.systems.addSystem<BossPatternSystem>();
        ecs.systems.addSystem<EnemySpawnSystem>();

        // Gameplay
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

    // --- Systèmes Communs ---
    ecs.systems.addSystem<DestructionSystem>();

    // --- Systèmes Visuels (Client & Standalone) ---
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
        bg.scroll_speed = _game_config.scroll_speed.value_or(60.f);

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
    if (env.isServer() || env.isClient()) {
        std::cout << "[GameManager] Multiplayer mode: Player spawning handled by network replication" << std::endl;
        return;
    }

    auto& ecs = env.getECS();
    float start_x = _player_config.start_x.value_or(150.0f);
    float start_y = _player_config.start_y.value_or(500.0f);

    _player = env.spawn<Player>(SpawnPolicy::PREDICTED, std::pair<float, float>{start_x, start_y});
    if (_player) {
        _player->setTexture(_player_config.sprite_path.value_or(""));

        // Animation setup
        Entity player_id = _player->getId();
        int num_frames = _player_config.animation_frames.value_or(5);
        float anim_speed = _player_config.animation_speed.value_or(0.1f);
        AnimationHelper::setupHorizontalAnimation(ecs.registry, player_id, _player_config, num_frames, anim_speed);

        // Scale
        if (_player_config.scale.has_value()) {
            _player->setScale({_player_config.scale.value(), _player_config.scale.value()});
        }

        _player->setFireRate(_player_config.fire_rate.value_or(0.25f));
        _player->setLifePoint(_player_config.hp.value_or(5));

        // Collision Tags from config
        for (const auto& tag : _player_config.collision_tags) {
            _player->addCollisionTag(tag);
        }

        // Default components
        ChargedShotComponent charged_shot;
        charged_shot.min_charge_time = _player_config.min_charge_time.value_or(0.5f);
        charged_shot.max_charge_time = _player_config.max_charge_time.value_or(2.0f);
        ecs.registry.addComponent<ChargedShotComponent>(_player->getId(), charged_shot);

        PlayerPodComponent player_pod;
        player_pod.has_pod = false;
        player_pod.pod_entity = -1;
        player_pod.pod_attached = false;
        player_pod.last_known_hp = _player_config.hp.value_or(5);
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
        spawn_comp.spawn_interval = _game_config.enemy_spawn_interval.value_or(2.0f);
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
        pod_spawn_comp.spawn_interval = _game_config.pod_spawn_interval.value_or(15.0f);
        pod_spawn_comp.min_spawn_interval = _game_config.pod_min_spawn_interval.value_or(10.0f);
        pod_spawn_comp.max_spawn_interval = _game_config.pod_max_spawn_interval.value_or(20.0f);
        pod_spawn_comp.can_spawn = true;
        ecs.registry.addComponent<PodSpawnComponent>(pod_spawner, pod_spawn_comp);
    }
}

void GameManager::initUI(Environment& env) {
    if (env.isServer()) return;

    auto& ecs = env.getECS();
    UIConfig ui;
    try {
        ui = ConfigLoader::loadUIConfig("src/RType/Common/content/config/ui.cfg");
    } catch (...) {
        std::cerr << "[GameManager] Failed to load ui.cfg, using default values" << std::endl;
    }

    // Timer UI
    auto& t = ui.elements["Timer"];
    _timerEntity = ecs.registry.createEntity();
    TextComponent timerText;
    timerText.text = "Time: 0s";
    timerText.fontPath = t.font.empty() ? "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf" : t.font;
    timerText.characterSize = static_cast<unsigned int>(t.size == 0 ? 28 : t.size);
    timerText.color = t.color;
    timerText.x = t.x;
    timerText.y = t.y;
    ecs.registry.addComponent<TextComponent>(_timerEntity, timerText);

    // Boss HP UI
    auto& b = ui.elements["BossHP"];
    _bossHPEntity = ecs.registry.createEntity();
    TextComponent bossText;
    bossText.text = "";
    bossText.fontPath = b.font.empty() ? "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf" : b.font;
    bossText.characterSize = static_cast<unsigned int>(b.size == 0 ? 28 : b.size);
    bossText.color = b.color;
    bossText.x = b.x;
    bossText.y = b.y;
    ecs.registry.addComponent<TextComponent>(_bossHPEntity, bossText);

    // Score tracker & Status
    _scoreTrackerEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<ScoreComponent>(_scoreTrackerEntity, {0, 0});

    _statusDisplayEntity = ecs.registry.createEntity();
    StatusDisplayComponent status;
    status.is_initialized = true;
    status.player_entity = -1;
    ecs.registry.addComponent<StatusDisplayComponent>(_statusDisplayEntity, status);

    // Charge Bar
    auto& cb = ui.elements["ChargeBar"];
    _chargeBarEntity = ecs.registry.createEntity();
    ChargeBarComponent chargeBar;
    chargeBar.x = cb.x;
    chargeBar.y = cb.y;
    chargeBar.bar_width = cb.width == 0 ? 200.0f : cb.width;
    chargeBar.bar_height = cb.height == 0 ? 20.0f : cb.height;
    ecs.registry.addComponent<ChargeBarComponent>(_chargeBarEntity, chargeBar);

    // Lives Display
    auto& ld = ui.elements["LivesDisplay"];
    _livesEntity = ecs.registry.createEntity();
    LivesDisplayComponent lives;
    lives.x = ld.x;
    lives.y = ld.y;
    lives.icon_size = ld.icon_size == 0 ? 32.0f : ld.icon_size;
    lives.icon_spacing = ld.icon_spacing == 0 ? 40.0f : ld.icon_spacing;
    ecs.registry.addComponent<LivesDisplayComponent>(_livesEntity, lives);

    // Score Display
    auto& sd = ui.elements["ScoreDisplay"];
    _scoreDisplayEntity = ecs.registry.createEntity();
    ScoreDisplayComponent score;
    score.digit_count = sd.digit_count == 0 ? 7 : sd.digit_count;
    score.x = sd.x;
    score.y = sd.y;
    ecs.registry.addComponent<ScoreDisplayComponent>(_scoreDisplayEntity, score);
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
