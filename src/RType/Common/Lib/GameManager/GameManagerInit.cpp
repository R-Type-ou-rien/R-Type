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
#include "src/RType/Common/Components/charged_shot.hpp"
#include "src/RType/Common/Systems/ai_behavior.hpp"
#include "src/RType/Common/Systems/score.hpp"
#include "src/RType/Common/Systems/animation_helper.hpp"
#include "src/RType/Common/Systems/pod_system.hpp"
#include "src/Engine/Lib/Systems/PatternSystem/PatternSystem.hpp"
#include "src/Engine/Lib/Systems/PlayerBoundsSystem.hpp"

void GameManager::initSystems(Environment& env) {
    auto& ecs = env.getECS();

    ecs.systems.addSystem<ShooterSystem>();
    ecs.systems.addSystem<Damage>();
    ecs.systems.addSystem<HealthSystem>();
    ecs.systems.addSystem<PatternSystem>();
    ecs.systems.addSystem<EnemySpawnSystem>();
    ecs.systems.addSystem<PodSystem>();
    ecs.systems.addSystem<AIBehaviorSystem>();
    ecs.systems.addSystem<BoundsSystem>();
    ecs.systems.addSystem<PlayerBoundsSystem>();
    ecs.systems.addSystem<ScoreSystem>();
}

void GameManager::initBackground(Environment& env) {
    if (!env.isServer()) {
        const std::string bgPath = "src/RType/Common/content/sprites/test.png";
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
            _player->setScale({_player_config.scale.value(), _player_config.scale.value()});
        }

        _player->setFireRate(_player_config.fire_rate.value());
        _player->setLifePoint(_player_config.hp.value());
        _player->setLifePoint(_player_config.hp.value());

        _player->addCollisionTag("AI");
        _player->addCollisionTag("ENEMY_PROJECTILE");
        _player->addCollisionTag("OBSTACLE");
        _player->addCollisionTag("ITEM");

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

void GameManager::initSpawner(Environment& env) {
    auto& ecs = env.getECS();

    if (!env.isClient()) {
        // Créer le timer de jeu
        Entity timer_entity = ecs.registry.createEntity();
        ecs.registry.addComponent<GameTimerComponent>(timer_entity, {0.0f});

        Entity spawner = ecs.registry.createEntity();
        EnemySpawnComponent spawn_comp;
        spawn_comp.spawn_interval = 2.0f;
        spawn_comp.is_active = true;
        ecs.registry.addComponent<EnemySpawnComponent>(spawner, spawn_comp);

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
        // HP UI
        _uiEntity = ecs.registry.createEntity();
        ecs.registry.addComponent<TextComponent>(
            _uiEntity,
            {"HP: " + std::to_string(_player_config.hp.value()),
             "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 28, sf::Color::White, 10, 10});

        // Score UI
        _scoreEntity = ecs.registry.createEntity();
        ecs.registry.addComponent<TextComponent>(
            _scoreEntity, {"Score: 0", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 28,
                           sf::Color::Yellow, 10, 50});

        // Timer UI
        _timerEntity = ecs.registry.createEntity();
        ecs.registry.addComponent<TextComponent>(
            _timerEntity, {"Time: 0s", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 28,
                           sf::Color::Cyan, 10, 90});

        // Score tracker
        _scoreTrackerEntity = ecs.registry.createEntity();
        ecs.registry.addComponent<ScoreComponent>(_scoreTrackerEntity, {0, 0});
    }
}
