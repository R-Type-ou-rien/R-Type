#include "GameManager.hpp"
#include <iostream>
#include <memory>
#include <ostream>
#include <algorithm>
#include <utility>

#include "Network/Network.hpp"
#include "src/Engine/Lib/Systems/CollisionSystem.hpp"
#include "src/RType/Common/Components/shooter.hpp"
#include "src/RType/Common/Components/damage.hpp"
#include "src/RType/Common/Components/spawn.hpp"
// #include "src/Engine/Lib/Systems/PatternSystem/PatternSystem.hpp"

GameManager::GameManager() {}

void GameManager::init(ECS& ecs) {
    ecs.systems.addSystem<ShooterSystem>();
    ecs._textureManager.load_resource("content/sprites/r-typesheet42.gif",
                                      sf::Texture("content/sprites/r-typesheet42.gif"));
    ecs.systems.addSystem<BoxCollision>();
    ecs._textureManager.load_resource("content/sprites/r-typesheet1.gif",
                                      sf::Texture("content/sprites/r-typesheet1.gif"));

    {
        const std::string bgPath = "content/sprites/background-R-Type.png";
        Entity bgEntity = ecs.registry.createEntity();

        BackgroundComponent bg{};
        bg.x_offset = 0.f;
        bg.scroll_speed = 60.f;

        if (ecs._textureManager.is_loaded(bgPath)) {
            bg.texture_handle = ecs._textureManager.get_handle(bgPath).value();
        } else {
            bg.texture_handle = ecs._textureManager.load_resource(bgPath, sf::Texture(bgPath));
        }

        ecs.registry.addComponent<BackgroundComponent>(bgEntity, bg);
    }
    auto enemy = std::make_unique<AI>(ecs, std::pair<float, float>(300.f, 300.f));
    enemy->setTexture("content/sprites/r-typesheet42.gif");
    enemy->setTextureDimension(rect{32, 0, 32, 16});
    enemy->setPattern({{500.f, 300.f}, {500.f, 500.f}, {300.f, 500.f}, {300.f, 300.f}});
    enemy->setPatternLoop(true);

    Entity entity = enemy->getId();
    ecs.registry.addComponent<NetworkIdentity>(entity, {static_cast<uint64_t>(entity), 0});  // 0 = Server Owned

    _ennemies.push_back(std::move(enemy));
}

void GameManager::loadInputSetting(ECS& ecs) {
    // Legacy function, inputs are now setup per player in setupPlayerInputs
}

void GameManager::onPlayerConnect(ECS& ecs, std::uint32_t id) {
    float y_pos = 100.f + ((id % 10) * 50.f);  // Clamp ID effect to avoid off-screen
    auto player = std::make_shared<Player>(ecs, std::pair<float, float>{100.f, y_pos});

    std::cout << "GameManager::onPlayerConnect called for ID: " << id << " Spawning at Y: " << y_pos << std::endl;

    player->setTexture("content/sprites/r-typesheet42.gif");
    player->setTextureDimension(rect{0, 0, 32, 16});
    player->setFireRate(0.5);

    // Assign NetworkIdentity so actions and updates are synced correctly
    Entity entity = player->getId();
    ecs.registry.addComponent<NetworkIdentity>(entity, {static_cast<uint64_t>(entity), id});

    setupPlayerInputs(ecs, *player);
    std::cout << "Player " << id << " spawned with entity " << entity << std::endl;
    _players[id] = player;
}

void GameManager::setupPlayerInputs(ECS& ecs, Player& player) {
    ecs.input.bindAction("move_left", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Q});
    player.bindActionCallbackPressed("move_left", [](Registry& registry, system_context context, Entity entity) {
        if (registry.hasComponent<Velocity2D>(entity)) {
            Velocity2D& vel = registry.getComponent<Velocity2D>(entity);
            vel.vx = -100.0f;
        }
    });
    player.bindActionCallbackOnReleased("move_left", [](Registry& registry, system_context context, Entity entity) {
        if (registry.hasComponent<Velocity2D>(entity)) {
            Velocity2D& vel = registry.getComponent<Velocity2D>(entity);
            vel.vx = 0;
        }
    });

    ecs.input.bindAction("move_right", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::D});
    player.bindActionCallbackPressed("move_right", [](Registry& registry, system_context context, Entity entity) {
        if (registry.hasComponent<Velocity2D>(entity)) {
            Velocity2D& vel = registry.getComponent<Velocity2D>(entity);
            vel.vx = +100.0f;
        }
    });
    player.bindActionCallbackOnReleased("move_right", [](Registry& registry, system_context context, Entity entity) {
        if (registry.hasComponent<Velocity2D>(entity)) {
            Velocity2D& vel = registry.getComponent<Velocity2D>(entity);
            vel.vx = 0;
        }
    });

    ecs.input.bindAction("move_up", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Z});
    player.bindActionCallbackPressed("move_up", [](Registry& registry, system_context context, Entity entity) {
        if (registry.hasComponent<Velocity2D>(entity)) {
            Velocity2D& vel = registry.getComponent<Velocity2D>(entity);
            vel.vy = -100.0f;
        }
    });
    player.bindActionCallbackOnReleased("move_up", [](Registry& registry, system_context context, Entity entity) {
        if (registry.hasComponent<Velocity2D>(entity)) {
            Velocity2D& vel = registry.getComponent<Velocity2D>(entity);
            vel.vy = 0;
        }
    });

    ecs.input.bindAction("move_down", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::S});
    player.bindActionCallbackPressed("move_down", [](Registry& registry, system_context context, Entity entity) {
        if (registry.hasComponent<Velocity2D>(entity)) {
            Velocity2D& vel = registry.getComponent<Velocity2D>(entity);
            vel.vy = +100.0f;
        }
    });
    player.bindActionCallbackOnReleased("move_down", [](Registry& registry, system_context context, Entity entity) {
        if (registry.hasComponent<Velocity2D>(entity)) {
            Velocity2D& vel = registry.getComponent<Velocity2D>(entity);
            vel.vy = 0;
        }
    });

    ecs.input.bindAction("shoot", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Space});
    player.bindActionCallbackPressed("shoot", [](Registry& registry, system_context context, Entity entity) {
        if (registry.hasComponent<ShooterComponent>(entity)) {
            ShooterComponent& shoot = registry.getComponent<ShooterComponent>(entity);
            shoot.is_shooting = true;
        }
    });
}

void GameManager::update(ECS& ecs) {}
