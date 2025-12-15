#include "GameManager.hpp"
#include <iostream>
#include <memory>
#include <ostream>
#include <algorithm>
#include <utility>
#include "src/RType/Common/Components/health.hpp"
#include "src/RType/Common/Components/shooter.hpp"
<<<<<<< HEAD
#include "src/RType/Common/Components/damage.hpp"
#include "src/RType/Common/Components/spawn.hpp"
#include "src/Engine/Lib/Systems/PatternSystem/PatternSystem.hpp"
=======
#include "Components/StandardComponents.hpp"
>>>>>>> 4e3429f (feat: map background and spawn mobs)

GameManager::GameManager() {}

void GameManager::init(ECS& ecs) {
    ecs.systems.addSystem<ShooterSystem>();
    ecs.systems.addSystem<Damage>();
    ecs.systems.addSystem<HealthSystem>();
    ecs.systems.addSystem<PatternSystem>();
    ecs.systems.addSystem<SpawnSystem>();

    {
        const std::string bgPath = "content/sprites/background-R-Type.png";
        Entity bgEntity = ecs.registry.createEntity();

        BackgroundComponent bg{};
        bg.x_offset = 0.f;
        bg.scroll_speed = 60.f;  // pixels per second, adjust if needed

        if (ecs._textureManager.is_loaded(bgPath)) {
            bg.texture_handle = ecs._textureManager.get_handle(bgPath).value();
        } else {
            bg.texture_handle = ecs._textureManager.load_resource(bgPath, sf::Texture(bgPath));
        }

        ecs.registry.addComponent<BackgroundComponent>(bgEntity, bg);
    }
    _player = std::make_unique<Player>(ecs, std::pair<float, float>{100.f, 300.f});
    _player->setTexture("content/sprites/r-typesheet42.gif");
    _player->setTextureDimension(rect{0, 0, 32, 16});
    _player->setFireRate(0.5);
    _player->setLifePoint(100);
    _player->addCollisionTag("AI");
    _player->addCollisionTag("ENEMY_PROJECTILE");
    _player->addCollisionTag("ITEM");
    _player->addCollisionTag("GROUND");

    loadInputSetting(ecs);
    auto enemy = std::make_unique<AI>(ecs, std::pair<float, float>(600.f, 200.f));
    enemy->setTextureEnemy("content/sprites/r-typesheet8.gif");
    enemy->setPatternType(PatternComponent::SINUSOIDAL);
    enemy->setLifePoint(10);
    enemy->setCurrentHealth(10);
    enemy->addCollisionTag("FRIENDLY_PROJECTILE");
    enemy->addCollisionTag("PLAYER");
    _ennemies.push_back(std::move(enemy));

    Entity spawner = ecs.registry.createEntity();
    SpawnComponent spawn_comp;
    spawn_comp.spawn_interval = 5.0f;
    spawn_comp.enemies_per_wave = 3;
    spawn_comp.is_active = true;
    ecs.registry.addComponent<SpawnComponent>(spawner, spawn_comp);

    _uiEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _uiEntity,
        {"HP: 100", "/usr/share/fonts/liberation-mono-fonts/LiberationMono-Regular.ttf", 30, sf::Color::White, 10, 10});

    return;
}

void GameManager::loadInputSetting(ECS& ecs) {
    ecs.input.bindAction("move_left", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Q});
    _player->bindActionCallbackPressed("move_left", [](Registry& registry, system_context context, Entity entity) {
        if (registry.hasComponent<Velocity2D>(entity)) {
            Velocity2D& vel = registry.getComponent<Velocity2D>(entity);
            vel.vx = -100.0f;
        }
    });
    _player->bindActionCallbackOnReleased("move_left", [](Registry& registry, system_context context, Entity entity) {
        if (registry.hasComponent<Velocity2D>(entity)) {
            Velocity2D& vel = registry.getComponent<Velocity2D>(entity);
            vel.vx = 0;
        }
    });

    ecs.input.bindAction("move_right", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::D});
    _player->bindActionCallbackPressed("move_right", [](Registry& registry, system_context context, Entity entity) {
        if (registry.hasComponent<Velocity2D>(entity)) {
            Velocity2D& vel = registry.getComponent<Velocity2D>(entity);
            vel.vx = +100.0f;
        }
    });
    _player->bindActionCallbackOnReleased("move_right", [](Registry& registry, system_context context, Entity entity) {
        if (registry.hasComponent<Velocity2D>(entity)) {
            Velocity2D& vel = registry.getComponent<Velocity2D>(entity);
            vel.vx = 0;
        }
    });

    ecs.input.bindAction("move_up", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Z});
    _player->bindActionCallbackPressed("move_up", [](Registry& registry, system_context context, Entity entity) {
        if (registry.hasComponent<Velocity2D>(entity)) {
            Velocity2D& vel = registry.getComponent<Velocity2D>(entity);
            vel.vy = -100.0f;
        }
    });
    _player->bindActionCallbackOnReleased("move_up", [](Registry& registry, system_context context, Entity entity) {
        if (registry.hasComponent<Velocity2D>(entity)) {
            Velocity2D& vel = registry.getComponent<Velocity2D>(entity);
            vel.vy = 0;
        }
    });

    ecs.input.bindAction("move_down", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::S});
    _player->bindActionCallbackPressed("move_down", [](Registry& registry, system_context context, Entity entity) {
        if (registry.hasComponent<Velocity2D>(entity)) {
            Velocity2D& vel = registry.getComponent<Velocity2D>(entity);
            vel.vy = +100.0f;
        }
    });
    _player->bindActionCallbackOnReleased("move_down", [](Registry& registry, system_context context, Entity entity) {
        if (registry.hasComponent<Velocity2D>(entity)) {
            Velocity2D& vel = registry.getComponent<Velocity2D>(entity);
            vel.vy = 0;
        }
    });

    ecs.input.bindAction("shoot", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Space});
    std::cout << "Player id " << _player->getId() << std::endl;
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
            shoot.is_shooting = false;
        }
    });
}

void GameManager::update(ECS& ecs) {
    if (_player) {
        Entity player_id = _player->getId();
        if (ecs.registry.hasComponent<HealthComponent>(player_id)) {
            int hp = _player->getCurrentHealth();
            if (ecs.registry.hasComponent<TextComponent>(_uiEntity)) {
                auto& text = ecs.registry.getComponent<TextComponent>(_uiEntity);
                text.text = "HP: " + std::to_string(hp);
            }
        } else {
            if (ecs.registry.hasComponent<TextComponent>(_uiEntity)) {
                auto& text = ecs.registry.getComponent<TextComponent>(_uiEntity);
                text.text = "GAME OVER";
            }
        }
    }
}
