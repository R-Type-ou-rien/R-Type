#include "GameManager.hpp"
#include <iostream>
#include <memory>
#include <ostream>
#include <algorithm>
#include <utility>
#include "src/RType/Common/Components/shooter.hpp"
#include "Components/StandardComponents.hpp"

GameManager::GameManager() {
}

void GameManager::init(ECS& ecs) {
    ecs.systems.addSystem<ShooterSystem>();

    {
        const std::string bgPath = "content/sprites/background-R-Type.png";
        Entity bgEntity = ecs.registry.createEntity();

        BackgroundComponent bg{};
        bg.x_offset = 0.f;
        bg.scroll_speed = 60.f; // pixels per second, adjust if needed

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
    loadInputSetting(ecs);
    auto enemy = std::make_unique<AI>(ecs, std::pair<float, float>(300.f, 300.f));
    enemy->setTexture("content/sprites/r-typesheet42.gif");
    enemy->setTextureDimension(rect{32, 0, 32, 16});
    enemy->setPattern({{500.f, 300.f}, {500.f, 500.f}, {300.f, 500.f}, {300.f, 300.f}});
    enemy->setPatternLoop(true);
    
    _ennemies.push_back(std::move(enemy));

    // Spawner par défaut: 1 mob toutes les 2s
    {
        Entity spawner = ecs.registry.createEntity();
        SpawnComponent sp{};
        sp.active = true;
        sp.interval = 2.0f;
        sp.sprite_path = "content/sprites/r-typesheet8.gif";
        sp.frame = rect{0, 0, 32, 32};
        sp.scale_x = 2.0f;
        sp.scale_y = 2.0f;
        sp.speed_x = -100.0f; // défile vers la gauche
        ecs.registry.addComponent<SpawnComponent>(spawner, sp);
    }

    // Démarrer le chrono du Boss (délai configurable via setBossTriggerTime)
    _bossClock.restart();
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
    // Evénement Boss après _bossTriggerSeconds (configurable)
    if (!_bossSpawned && _bossClock.getElapsedTime().asSeconds() >= _bossTriggerSeconds) {
        // 1) Désactiver tous les spawners
        const auto& spawners = ecs.registry.getEntities<SpawnComponent>();
        for (Entity e : spawners) {
            auto& sp = ecs.registry.getComponent<SpawnComponent>(e);
            sp.active = false;
        }

        // 2) Supprimer toutes les entités ENEMY (tag)
        const auto& tagged = ecs.registry.getEntities<TagComponent>();
        for (Entity e : tagged) {
            const auto& tag = ecs.registry.getComponent<TagComponent>(e).tags;
            if (std::find(tag.begin(), tag.end(), std::string("ENEMY")) != tag.end()) {
                ecs.registry.destroyEntity(e);
            }
        }

        // 3) Créer le Boss statique
        _boss = std::make_unique<AI>(ecs, std::pair<float, float>{700.f, 300.f});
        _boss->setTexture(_bossSpritePath);
        _boss->setTextureDimension(_bossFrame);
        _boss->setScale(_bossScale);
        _boss->setDisplayLayer(10);
        _boss->addTag("BOSS");
        if (_bossFireRate > 0.0)
            _boss->setFireRate(_bossFireRate);

        _bossSpawned = true;
        std::cout << "[GameManager] Boss apparu après " << _bossTriggerSeconds << " secondes !" << std::endl;
    }
}