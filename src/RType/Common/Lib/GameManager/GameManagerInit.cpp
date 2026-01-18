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
#include "src/Engine/Lib/Components/LobbyIdComponent.hpp"
#include "src/RType/Common/Systems/behavior.hpp"
#include "src/RType/Common/Systems/score.hpp"
#include "src/RType/Common/Systems/animation_helper.hpp"
#include "src/RType/Common/Systems/powerup.hpp"
#include "src/RType/Common/Systems/boss_system.hpp"
#include "src/RType/Common/Systems/boss_tail.hpp"
#include "src/RType/Common/Systems/projectile_cleanup.hpp"
#include "src/RType/Common/Systems/pod_system.hpp"
#include "src/RType/Common/Systems/status_display.hpp"
#include "src/RType/Common/Systems/leaderboard_system.hpp"
#include "src/RType/Common/Systems/game_state_system.hpp"
#include "src/RType/Common/Systems/level_transition.hpp"
#include "src/Engine/Lib/Systems/PatternSystem/PatternSystem.hpp"
#include "src/Engine/Lib/Systems/PlayerBoundsSystem.hpp"
#include "src/Engine/Core/Scene/SceneLoader.hpp"
#include "src/RType/Common/Scene/ScenePrefabs.hpp"
#include "CollisionSystem.hpp"

#include "src/Engine/Lib/Systems/PhysicsSystem.hpp"
#include "src/Engine/Lib/Systems/ActionScriptSystem.hpp"
#include "src/Engine/Lib/Systems/DestructionSystem.hpp"

void GameManager::initSystems(std::shared_ptr<Environment> env) {
    auto& ecs = env->getECS();

    // Game logic systems - server only (NOT client)
    if (!env->isClient()) {
        ecs.systems.addSystem<BoxCollision>();
        ecs.systems.addSystem<ShooterSystem>();
        ecs.systems.addSystem<Damage>();
        ecs.systems.addSystem<HealthSystem>();
        ecs.systems.addSystem<ProjectileCleanupSystem>();
        ecs.systems.addSystem<BehaviorSystem>();
        ecs.systems.addSystem<PatternSystem>();
        ecs.systems.addSystem<BossSystem>();
        ecs.systems.addSystem<EnemySpawnSystem>();
        ecs.systems.addSystem<PodSystem>();
        ecs.systems.addSystem<BossTailSystem>();
        ecs.systems.addSystem<BoundsSystem>();
        ecs.systems.addSystem<PlayerBoundsSystem>();
        ecs.systems.addSystem<ScoreSystem>();
        ecs.systems.addSystem<GameStateSystem>();
        ecs.systems.addSystem<PhysicsSystem>();
        ecs.systems.addSystem<ActionScriptSystem>();
        ecs.systems.addSystem<WallCollisionSystem>();
    }

    ecs.systems.addSystem<DestructionSystem>();

    ecs.systems.addSystem<DestructionSystem>();

    if (!env->isServer()) {
        std::cout << "[GameManager] Registering Client Systems (Leaderboard, Status, Transition)" << std::endl;
        ecs.systems.addSystem<StatusDisplaySystem>();
        ecs.systems.addSystem<LeaderboardSystem>();
        ecs.systems.addSystem<LevelTransitionSystem>();
    }
}

void GameManager::initBackground(std::shared_ptr<Environment> env, const LevelConfig& config) {
    if (!env->isServer()) {
        std::string bgPath = config.background_texture;
        if (bgPath.empty()) {
            bgPath = "src/RType/Common/content/sprites/test.png";
        }

        auto& ecs = env->getECS();
        Entity bgEntity = ecs.registry.createEntity();

        BackgroundComponent bg{};
        bg.x_offset = 0.f;
        bg.scroll_speed = _game_config.scroll_speed.value_or(60.f);

        bg.texture_handle = env->loadTexture(bgPath);

        ecs.registry.addComponent<BackgroundComponent>(bgEntity, bg);

        TagComponent tag;
        tag.tags.push_back("BACKGROUND");
        ecs.registry.addComponent<TagComponent>(bgEntity, tag);
    }
}

void GameManager::initBounds(std::shared_ptr<Environment> env) {
    WorldBoundsComponent bounds;
    auto& ecs = env->getECS();

    _boundsEntity = ecs.registry.createEntity();
    bounds.min_x = 0.0f;
    bounds.min_y = 0.0f;

    ecs.registry.addComponent<WorldBoundsComponent>(_boundsEntity, bounds);
}

void GameManager::initPlayer(std::shared_ptr<Environment> env) {
    if (env->isServer() || env->isClient()) {
        return;
    }

    auto& ecs = env->getECS();
    float start_x = _player_config.start_x.value_or(150.0f);
    float start_y = _player_config.start_y.value_or(500.0f);

    _player = env->spawn<Player>(SpawnPolicy::PREDICTED, std::pair<float, float>{start_x, start_y}, _player_config);
    if (_player) {
        _player->setTexture(_player_config.sprite_path.value_or(""));
        Entity player_id = _player->getId();
        int num_frames = _player_config.animation_frames.value_or(5);
        float anim_speed = _player_config.animation_speed.value_or(0.1f);
        AnimationHelper::setupHorizontalAnimation(ecs.registry, player_id, _player_config, num_frames, anim_speed);

        if (_player_config.scale.has_value()) {
            _player->setScale({_player_config.scale.value(), _player_config.scale.value()});
        }

        for (const auto& tag : _player_config.collision_tags) {
            _player->addCollisionTag(tag);
        }

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
        ecs.registry.addComponent<ScoreComponent>(_player->getId(), {0, 0});
    }
}

void GameManager::initSpawner(std::shared_ptr<Environment> env, const LevelConfig& config) {
    auto& ecs = env->getECS();

    // Use the current lobby ID for entity tagging
    uint32_t lobbyId = _currentLobbyId;

    if (!env->isClient()) {
        Entity timer_entity = ecs.registry.createEntity();
        ecs.registry.addComponent<GameTimerComponent>(timer_entity, {0.0f});
        NetworkIdentity timer_net_id;
        timer_net_id.guid = timer_entity;
        timer_net_id.ownerId = 0;
        ecs.registry.addComponent<NetworkIdentity>(timer_entity, timer_net_id);
        if (lobbyId != 0) {
            ecs.registry.addComponent<LobbyIdComponent>(timer_entity, {lobbyId});
        }

        Entity spawner = ecs.registry.createEntity();
        EnemySpawnComponent spawn_comp;
        spawn_comp.spawn_interval = _game_config.enemy_spawn_interval.value_or(2.0f);
        spawn_comp.is_active = true;
        spawn_comp.enemies_config_path = config.enemies_config;
        spawn_comp.boss_config_path = config.boss_config;
        spawn_comp.boss_section = config.boss_section;
        spawn_comp.game_config_path = config.game_config;
        spawn_comp.lobby_id = lobbyId;  // Set lobby ID for spawned entities

        ecs.registry.addComponent<EnemySpawnComponent>(spawner, spawn_comp);
        ecs.registry.addComponent<NetworkIdentity>(spawner, {static_cast<uint32_t>(spawner), 0});
        if (lobbyId != 0) {
            ecs.registry.addComponent<LobbyIdComponent>(spawner, {lobbyId});
        }

        Entity scripted_spawner = ecs.registry.createEntity();
        ScriptedSpawnComponent scripted_spawn_comp;
        scripted_spawn_comp.script_path = config.spawn_script;
        ecs.registry.addComponent<ScriptedSpawnComponent>(scripted_spawner, scripted_spawn_comp);
        ecs.registry.addComponent<EnemySpawnComponent>(scripted_spawner,
                                                       spawn_comp);  // Associate with spawn_comp for lobby_id
        if (lobbyId != 0) {
            ecs.registry.addComponent<LobbyIdComponent>(scripted_spawner, {lobbyId});
        }

        Entity pod_spawner = ecs.registry.createEntity();
        PodSpawnComponent pod_spawn_comp;
        pod_spawn_comp.spawn_interval = _game_config.pod_spawn_interval.value_or(15.0f);
        pod_spawn_comp.min_spawn_interval = _game_config.pod_min_spawn_interval.value_or(10.0f);
        pod_spawn_comp.max_spawn_interval = _game_config.pod_max_spawn_interval.value_or(20.0f);
        pod_spawn_comp.can_spawn = true;
        ecs.registry.addComponent<PodSpawnComponent>(pod_spawner, pod_spawn_comp);
        if (lobbyId != 0) {
            ecs.registry.addComponent<LobbyIdComponent>(pod_spawner, {lobbyId});
        }
    }
}

void GameManager::initUI(std::shared_ptr<Environment> env) {
    if (env->isServer())
        return;

    auto& ecs = env->getECS();
    UIConfig ui;
    try {
        std::string ui_path =
            _master_config.ui_config.empty() ? "src/RType/Common/content/config/ui.cfg" : _master_config.ui_config;
        ui = ConfigLoader::loadUIConfig(ui_path);
    } catch (const std::exception& e) {
        std::cerr << "[GameManager] Failed to load UI config: " << e.what() << ", using default values" << std::endl;
    }

    UIStatus layout(1920.0f, 1080.0f);

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

    _scoreTrackerEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<ScoreComponent>(_scoreTrackerEntity, {0, 0});

    _statusDisplayEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<StatusDisplayComponent>(_statusDisplayEntity,
                                                      StatusDisplayFactory::createStatusDisplay());

    _chargeBarEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<ChargeBarComponent>(_chargeBarEntity, StatusDisplayFactory::createChargeBar(layout, ui));

    _livesEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<LivesDisplayComponent>(_livesEntity,
                                                     StatusDisplayFactory::createLivesDisplay(layout, ui));

    _scoreDisplayEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<ScoreDisplayComponent>(_scoreDisplayEntity,
                                                     StatusDisplayFactory::createScoreDisplay(layout, ui));
}

void GameManager::initScene(std::shared_ptr<Environment> env, const LevelConfig& config) {
    // Only server loads the scene - clients receive entities via network replication
    if (env->isClient()) {
        return;
    }

    auto& ecs = env->getECS();

    _scene_manager = std::make_unique<SceneManager>(ecs.registry);

    // Set the current lobby ID for scene entities
    if (_currentLobbyId != 0) {
        _scene_manager->setCurrentLobbyId(_currentLobbyId);
    }

    ScenePrefabs::registerAll(*_scene_manager, env->getTextureManager());

    try {
        _scene_manager->loadScene(config);

    } catch (const std::exception& e) {
        std::cerr << "GameManager: Failed to load scene: " << e.what() << std::endl;
    }
}

std::shared_ptr<Player> GameManager::createPlayerForClient(std::shared_ptr<Environment> env, uint32_t clientId, float x,
                                                           float y) {
    auto& ecs = env->getECS();

    auto newPlayer = env->spawn<Player>(SpawnPolicy::PREDICTED, std::pair<float, float>{x, y}, _player_config);
    if (!newPlayer) {
        std::cerr << "[GameManager] Failed to spawn player for client " << clientId << std::endl;
        return nullptr;
    }

    newPlayer->setTexture(_player_config.sprite_path.value_or("src/RType/Common/content/sprites/r-typesheet42.gif"));
    newPlayer->setTextureDimension({static_cast<float>(_player_config.sprite_x.value_or(0)),
                                    static_cast<float>(_player_config.sprite_y.value_or(0)),
                                    static_cast<float>(_player_config.sprite_w.value_or(33)),
                                    static_cast<float>(_player_config.sprite_h.value_or(17))});

    if (_player_config.scale.has_value()) {
        newPlayer->setScale({_player_config.scale.value(), _player_config.scale.value()});
    } else {
        newPlayer->setScale({2.5f, 2.5f});
    }

    for (const auto& tag : _player_config.collision_tags) {
        newPlayer->addCollisionTag(tag);
    }

    // Setup animation
    AnimationHelper::setupAnimation(ecs.registry, newPlayer->getId(), 0, 0, 33, 17, 5, 0.2f, 0.0f,
                                    AnimationMode::PingPong);

    // Add NetworkIdentity for replication and input handling
    ecs.registry.addComponent<NetworkIdentity>(newPlayer->getId(), {newPlayer->getId(), clientId});

    // Add LobbyIdComponent for lobby isolation
    if (_currentLobbyId != 0) {
        ecs.registry.addComponent<LobbyIdComponent>(newPlayer->getId(), {_currentLobbyId});
    }

    // Add ChargedShotComponent
    ChargedShotComponent charged_shot;
    charged_shot.min_charge_time = _player_config.min_charge_time.value_or(0.5f);
    charged_shot.max_charge_time = _player_config.max_charge_time.value_or(2.0f);
    ecs.registry.addComponent<ChargedShotComponent>(newPlayer->getId(), charged_shot);

    ProjectileConfigComponent defaults;
    ProjectileConfigComponent proj_config{
        .projectile_sprite = _player_config.projectile_sprite.value_or(defaults.projectile_sprite),
        .projectile_sprite_x = _player_config.projectile_sprite_x.value_or(defaults.projectile_sprite_x),
        .projectile_sprite_y = _player_config.projectile_sprite_y.value_or(defaults.projectile_sprite_y),
        .projectile_sprite_w = _player_config.projectile_sprite_w.value_or(defaults.projectile_sprite_w),
        .projectile_sprite_h = _player_config.projectile_sprite_h.value_or(defaults.projectile_sprite_h),
        .charged_sprite = _player_config.charged_sprite.value_or(defaults.charged_sprite),
        .charged_sprite_x = _player_config.charged_sprite_x.value_or(defaults.charged_sprite_x),
        .charged_sprite_y = _player_config.charged_sprite_y.value_or(defaults.charged_sprite_y),
        .charged_sprite_w = _player_config.charged_sprite_w.value_or(defaults.charged_sprite_w),
        .charged_sprite_h = _player_config.charged_sprite_h.value_or(defaults.charged_sprite_h),
    };
    ecs.registry.addComponent<ProjectileConfigComponent>(newPlayer->getId(), proj_config);

    // Add PlayerPodComponent
    PlayerPodComponent player_pod;
    player_pod.has_pod = false;
    player_pod.pod_entity = -1;
    player_pod.pod_attached = false;
    player_pod.last_known_hp = _player_config.hp.value_or(5);
    ecs.registry.addComponent<PlayerPodComponent>(newPlayer->getId(), player_pod);

    // Add ScoreComponent
    ScoreComponent playerScore{0, 0};
    ecs.registry.addComponent<ScoreComponent>(newPlayer->getId(), playerScore);

    return newPlayer;
}
