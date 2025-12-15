#include "GameManager.hpp"
#include <iostream>
#include <memory>
#include <ostream>
#include <utility>
#include "src/RType/Common/Components/shooter.hpp"

GameManager::GameManager() {
}

void GameManager::init(ECS& ecs) {
    ecs.systems.addSystem<ShooterSystem>();
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
        }
    });
}

void GameManager::update(ECS& ecs) {
}