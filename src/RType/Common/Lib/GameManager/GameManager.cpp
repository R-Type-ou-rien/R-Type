#include "GameManager.hpp"
#include <iostream>
#include <memory>
#include <ostream>
#include <utility>
#include "InputConfig.hpp"
#include "ResourceConfig.hpp"
#include "src/RType/Common/Components/health.hpp"
#include "src/RType/Common/Components/shooter.hpp"
#include "src/RType/Common/Components/damage.hpp"
#include "src/RType/Common/Components/spawn.hpp"
#include "src/RType/Common/Components/charged_shot.hpp"
#include "src/Engine/Lib/Systems/PatternSystem/PatternSystem.hpp"

GameManager::GameManager() {}

void GameManager::initSystems(ECS& ecs) {
    ecs.systems.addSystem<ShooterSystem>();
    ecs.systems.addSystem<Damage>();
    ecs.systems.addSystem<HealthSystem>();
    ecs.systems.addSystem<PatternSystem>();
    ecs.systems.addSystem<EnemySpawnSystem>();
}

void GameManager::initBackground(ECS& ecs, ResourceManager<TextureAsset>& textures) {
    const std::string bgPath = "content/sprites/background-R-Type.png";
    Entity bgEntity = ecs.registry.createEntity();

    BackgroundComponent bg{};
    bg.x_offset = 0.f;
    bg.scroll_speed = 60.f;

    if (textures.is_loaded(bgPath)) {
        bg.texture_handle = textures.get_handle(bgPath).value();
    } else {
        bg.texture_handle = textures.load(bgPath, TextureAsset(bgPath));
    }

    ecs.registry.addComponent<BackgroundComponent>(bgEntity, bg);
}

void GameManager::initPlayer(ECS& ecs, ResourceManager<TextureAsset>& textures) {
    _player = std::make_unique<Player>(ecs, std::pair<float, float>{PLAYER_START_X, PLAYER_START_Y}, textures);
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

void GameManager::initEnemies(ECS& ecs, ResourceManager<TextureAsset>& textures) {
    auto enemy = std::make_unique<AI>(ecs, std::pair<float, float>(600.f, 200.f), textures);
    enemy->setTextureEnemy("content/sprites/r-typesheet8.gif");
    enemy->setPatternType(PatternComponent::SINUSOIDAL);
    enemy->setLifePoint(10);
    enemy->setCurrentHealth(10);
    enemy->addCollisionTag("FRIENDLY_PROJECTILE");
    enemy->addCollisionTag("PLAYER");
    _ennemies.push_back(std::move(enemy));
}

void GameManager::initSpawner(ECS& ecs) {
    Entity spawner = ecs.registry.createEntity();
    EnemySpawnComponent spawn_comp;
    spawn_comp.spawn_interval = 5.0f;
    spawn_comp.enemies_per_wave = 3;
    spawn_comp.is_active = true;
    ecs.registry.addComponent<EnemySpawnComponent>(spawner, spawn_comp);
}

void GameManager::initUI(ECS& ecs) {
    _uiEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _uiEntity, {"HP: 100", "content/open_dyslexic/OpenDyslexic-Regular.otf", 30, sf::Color::White, 10, 10});

void GameManager::init(ECS& ecs, InputManager& inputs, ResourceManager<TextureAsset>& textures) {
    initSystems(ecs);
    
    initBackground(ecs, textures);
    initPlayer(ecs, textures);
    initEnemies(ecs, textures);
    initSpawner(ecs);
    initUI(ecs);
    
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

void GameManager::update(ECS& ecs, InputManager& inputs, ResourceManager<TextureAsset>& textures) {
    if (_player) {
        Entity player_id = _player->getId();
        if (ecs.registry.hasComponent<HealthComponent>(player_id)) {
            int hp = _player->getCurrentHealth();
            //     if (ecs.registry.hasComponent<TextComponent>(_uiEntity)) {
            //         auto& text = ecs.registry.getComponent<TextComponent>(_uiEntity);
            //         text.text = "HP: " + std::to_string(hp);
            //     }
            // } else {
            //     if (ecs.registry.hasComponent<TextComponent>(_uiEntity)) {
            //         auto& text = ecs.registry.getComponent<TextComponent>(_uiEntity);
            //         text.text = "GAME OVER";
            //     }
        }
}

void GameManager::loadInputSetting(InputManager& inputs) {
    setupMovementControls(inputs);
    setupShootingControls(inputs);
}

void GameManager::updateUI(ECS& ecs) {
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

void GameManager::update(ECS& ecs, InputManager& inputs, ResourceManager<TextureAsset>& textures) {
    updateUI(ecs);
        // vagues d'ennemis
    // check de victoire/défaite
    // power-ups
}
