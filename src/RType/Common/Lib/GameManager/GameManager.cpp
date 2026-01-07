#include "GameManager.hpp"
#include <iostream>
#include <memory>
#include <ostream>
#include <utility>
#include "ECS.hpp"
#include "GameEngineBase.hpp"
#include "InputConfig.hpp"
#include "ResourceConfig.hpp"
#include "src/RType/Common/Components/health.hpp"
#include "src/RType/Common/Components/shooter.hpp"
#include "src/RType/Common/Components/damage.hpp"
#include "src/RType/Common/Components/spawn.hpp"
#include "src/RType/Common/Components/charged_shot.hpp"
#include "src/Engine/Lib/Systems/PatternSystem/PatternSystem.hpp"

GameManager::GameManager() {}

void GameManager::initSystems(Environment& env) {
    auto& ecs = env.getECS();

    ecs.systems.addSystem<ShooterSystem>();
    ecs.systems.addSystem<Damage>();
    ecs.systems.addSystem<HealthSystem>();
    ecs.systems.addSystem<PatternSystem>();
    ecs.systems.addSystem<EnemySpawnSystem>();
}

void GameManager::initBackground(Environment& env) {
    /**
        env.isServer() return true si c'est le server qui
        'lit' ce code
    */
    if (!env.isServer()) {
        const std::string bgPath = "content/sprites/background-R-Type.png";
        auto& ecs = env.getECS();
        Entity bgEntity = ecs.registry.createEntity();

        BackgroundComponent bg{};
        bg.x_offset = 0.f;
        bg.scroll_speed = 60.f;

        bg.texture_handle = env.loadTexture(bgPath);

        ecs.registry.addComponent<BackgroundComponent>(bgEntity, bg);
    }
}

void GameManager::initPlayer(Environment& env) {
    auto& ecs = env.getECS();
    /**
        Crée un actor en suivant la policy donné en parametre.

        Ici PREDICTED signifie que le player est crée à la fois sur le serveur
        et le client afin que le client puisse prédir les evenements liés au player
    */
    _player = env.spawn<Player>(SpawnPolicy::PREDICTED, std::pair<float, float>{PLAYER_START_X, PLAYER_START_Y});
    if (_player) {
        _player->setTexture("content/sprites/r-typesheet42.gif");
        _player->setTextureDimension(rect{0, 0, 32, 16});
        _player->setFireRate(0.5);
        _player->setLifePoint(PLAYER_MAX_HP);
        
        _player->addCollisionTag("AI");
        _player->addCollisionTag("ENEMY_PROJECTILE");
        _player->addCollisionTag("ITEM");
        _player->addCollisionTag("GROUND");
        
        ChargedShotComponent charged_shot;
        charged_shot.min_charge_time = 0.5f;
        charged_shot.max_charge_time = 2.0f;
        ecs.registry.addComponent<ChargedShotComponent>(_player->getId(), charged_shot);
    }
}

void GameManager::initEnemies(Environment& env) {
    /**
        Crée un actor en suivant la policy donné en parametre.

        Ici AUTHORITATIVE signifie que l'enemy est crée sur le serveur
        uniquement. Il n'existe donc pas lorsque le client 'lit' ce code
        d'où le if (enemy)
    */
    auto enemy = env.spawn<AI>(SpawnPolicy::AUTHORITATIVE, std::pair<float, float>(600.f, 200.f));

    if (enemy) {
        enemy->setTextureEnemy("content/sprites/r-typesheet8.gif");
        enemy->setPatternType(PatternComponent::SINUSOIDAL);
        enemy->setLifePoint(10);
        enemy->setCurrentHealth(10);
        enemy->addCollisionTag("FRIENDLY_PROJECTILE");
        enemy->addCollisionTag("PLAYER");
        _ennemies.push_back(std::move(enemy));
    }
}

void GameManager::initSpawner(Environment& env) {
    auto& ecs = env.getECS();

    /**
        env.isClient() return true si c'est le client qui
        'lit' ce code
    */
    if (!env.isClient()) {
        Entity spawner = ecs.registry.createEntity();
        EnemySpawnComponent spawn_comp;
        spawn_comp.spawn_interval = 5.0f;
        spawn_comp.enemies_per_wave = 3;
        spawn_comp.is_active = true;
        ecs.registry.addComponent<EnemySpawnComponent>(spawner, spawn_comp);
    }
}

void GameManager::initUI(Environment& env) {
    auto& ecs = env.getECS();

    /**
        env.isClient() return true si c'est le client qui
        'lit' ce code (il existe le mode STANDALONE pour tout lancer en local)
    */
    if (!env.isServer()) {
        _uiEntity = ecs.registry.createEntity();
        ecs.registry.addComponent<TextComponent>(
            _uiEntity, {"HP: 100", "content/open_dyslexic/OpenDyslexic-Regular.otf", 30, sf::Color::White, 10, 10});
    }
}

void GameManager::init(Environment& env, InputManager& inputs) {
    initSystems(env);

    env.loadSound("shoot", "content/sounds/shoot.wav");
    env.loadSound("charg_start", "content/sounds/charge_start.wav");
    env.loadSound("charg_loop", "content/sounds/charge_loop.wav");

    initBackground(env);
    initPlayer(env);
    initEnemies(env);
    initSpawner(env);
    initUI(env);
    
    loadInputSetting(inputs);
}

void GameManager::setupMovementControls(InputManager& inputs) {
    inputs.bindAction("move_left", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Left});
    _player->bindActionCallbackPressed("move_left", [](Registry& registry, system_context context, Entity entity) {
        if (registry.hasComponent<Velocity2D>(entity)) {
            registry.getComponent<Velocity2D>(entity).vx = -GameManager::PLAYER_SPEED;
        }
    });
    _player->bindActionCallbackOnReleased("move_left", [](Registry& registry, system_context context, Entity entity) {
        if (registry.hasComponent<Velocity2D>(entity)) {
            registry.getComponent<Velocity2D>(entity).vx = 0.0f;
        }
    });

    inputs.bindAction("move_right", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Right});
    _player->bindActionCallbackPressed("move_right", [](Registry& registry, system_context context, Entity entity) {
        if (registry.hasComponent<Velocity2D>(entity)) {
            registry.getComponent<Velocity2D>(entity).vx = GameManager::PLAYER_SPEED;
        }
    });
    _player->bindActionCallbackOnReleased("move_right", [](Registry& registry, system_context context, Entity entity) {
        if (registry.hasComponent<Velocity2D>(entity)) {
            registry.getComponent<Velocity2D>(entity).vx = 0.0f;
        }
    });

    inputs.bindAction("move_up", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Up});
    _player->bindActionCallbackPressed("move_up", [](Registry& registry, system_context context, Entity entity) {
        if (registry.hasComponent<Velocity2D>(entity)) {
            registry.getComponent<Velocity2D>(entity).vy = -GameManager::PLAYER_SPEED;
        }
    });
    _player->bindActionCallbackOnReleased("move_up", [](Registry& registry, system_context context, Entity entity) {
        if (registry.hasComponent<Velocity2D>(entity)) {
            registry.getComponent<Velocity2D>(entity).vy = 0.0f;
        }
    });

    inputs.bindAction("move_down", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Down});
    _player->bindActionCallbackPressed("move_down", [](Registry& registry, system_context context, Entity entity) {
        if (registry.hasComponent<Velocity2D>(entity)) {
            registry.getComponent<Velocity2D>(entity).vy = GameManager::PLAYER_SPEED;
        }
    });
    _player->bindActionCallbackOnReleased("move_down", [](Registry& registry, system_context context, Entity entity) {
        if (registry.hasComponent<Velocity2D>(entity)) {
            registry.getComponent<Velocity2D>(entity).vy = 0.0f;
        }
    });
}

void GameManager::setupShootingControls(InputManager& inputs) {

    // Tir
    inputs.bindAction("shoot", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Space});
    _player->bindActionCallbackPressed("shoot", [](Registry& registry, system_context context, Entity entity) {
        if (registry.hasComponent<ShooterComponent>(entity)) {
            ShooterComponent& shoot = registry.getComponent<ShooterComponent>(entity);
            shoot.is_shooting = true;
            shoot.trigger_pressed = true;
        }
    });
    _player->bindActionCallbackOnReleased("shoot", [](Registry& registry, system_context context, Entity entity) {
        if (registry.hasComponent<ShooterComponent>(entity)) {
            auto& shoot = registry.getComponent<ShooterComponent>(entity);
            shoot.trigger_pressed = false;
            
            if (registry.hasComponent<ChargedShotComponent>(entity)) {
                auto& charged = registry.getComponent<ChargedShotComponent>(entity);
                if (!charged.is_charging) {
                    shoot.is_shooting = false;
                }
            } else {
                shoot.is_shooting = false;
            }
        }
    });
}

void GameManager::loadInputSetting(InputManager& inputs) {
    setupMovementControls(inputs);
    setupShootingControls(inputs);
}

void GameManager::updateUI(Environment& env) {
    auto& ecs = env.getECS();

    if (!env.isServer()) {
        if (!_player || !ecs.registry.hasComponent<TextComponent>(_uiEntity)) {
            return;
        }
        auto& text = ecs.registry.getComponent<TextComponent>(_uiEntity);
        Entity player_id = _player->getId();
        if (ecs.registry.hasComponent<HealthComponent>(player_id)) {
            int hp = _player->getCurrentHealth();
            text.text = "HP: " + std::to_string(hp);
        } else {
            text.text = "GAME OVER";
        }
    }
}

void GameManager::update(Environment& env, InputManager& inputs) {
    updateUI(env);
        // vagues d'ennemis
    // check de victoire/défaite
    // power-ups
}
