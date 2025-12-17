#include "GameManager.hpp"
#include <iostream>
#include <memory>
#include <ostream>
#include <utility>
#include "Network/Network.hpp"
#include "src/RType/Common/Components/shooter.hpp"

GameManager::GameManager() {}

void GameManager::init(ECS& ecs) {
    ecs.systems.addSystem<ShooterSystem>();
    ecs._textureManager.load_resource("content/sprites/r-typesheet42.gif",
                                      sf::Texture("content/sprites/r-typesheet42.gif"));
    ecs._textureManager.load_resource("content/sprites/r-typesheet1.gif",
                                      sf::Texture("content/sprites/r-typesheet1.gif"));

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
    auto sendMessage = [](system_context& context, const std::string& action, bool pressed) {
        if (context.network_client.has_value()) {
            network::message<GameEvents> msg;
            msg.header.id = GameEvents::C_INPUT;
            msg << action << pressed;
            context.network_client.value().get().Send(msg);
        }
    };

    ecs.input.bindAction("move_left", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Q});
    _player->bindActionCallbackPressed("move_left",
                                       [sendMessage](Registry& registry, system_context context, Entity entity) {
                                           sendMessage(context, "move_left", true);
                                           if (registry.hasComponent<Velocity2D>(entity)) {
                                               Velocity2D& vel = registry.getComponent<Velocity2D>(entity);
                                               vel.vx = -100.0f;
                                           }
                                       });
    _player->bindActionCallbackOnReleased("move_left",
                                          [sendMessage](Registry& registry, system_context context, Entity entity) {
                                              sendMessage(context, "move_left", false);
                                              if (registry.hasComponent<Velocity2D>(entity)) {
                                                  Velocity2D& vel = registry.getComponent<Velocity2D>(entity);
                                                  vel.vx = 0;
                                              }
                                          });

    ecs.input.bindAction("move_right", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::D});
    _player->bindActionCallbackPressed("move_right",
                                       [sendMessage](Registry& registry, system_context context, Entity entity) {
                                           sendMessage(context, "move_right", true);
                                           if (registry.hasComponent<Velocity2D>(entity)) {
                                               Velocity2D& vel = registry.getComponent<Velocity2D>(entity);
                                               vel.vx = +100.0f;
                                           }
                                       });
    _player->bindActionCallbackOnReleased("move_right",
                                          [sendMessage](Registry& registry, system_context context, Entity entity) {
                                              sendMessage(context, "move_right", false);
                                              if (registry.hasComponent<Velocity2D>(entity)) {
                                                  Velocity2D& vel = registry.getComponent<Velocity2D>(entity);
                                                  vel.vx = 0;
                                              }
                                          });

    ecs.input.bindAction("move_up", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Z});
    _player->bindActionCallbackPressed("move_up",
                                       [sendMessage](Registry& registry, system_context context, Entity entity) {
                                           sendMessage(context, "move_up", true);
                                           if (registry.hasComponent<Velocity2D>(entity)) {
                                               Velocity2D& vel = registry.getComponent<Velocity2D>(entity);
                                               vel.vy = -100.0f;
                                           }
                                       });
    _player->bindActionCallbackOnReleased("move_up",
                                          [sendMessage](Registry& registry, system_context context, Entity entity) {
                                              sendMessage(context, "move_up", false);
                                              if (registry.hasComponent<Velocity2D>(entity)) {
                                                  Velocity2D& vel = registry.getComponent<Velocity2D>(entity);
                                                  vel.vy = 0;
                                              }
                                          });

    ecs.input.bindAction("move_down", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::S});
    _player->bindActionCallbackPressed("move_down",
                                       [sendMessage](Registry& registry, system_context context, Entity entity) {
                                           sendMessage(context, "move_down", true);
                                           if (registry.hasComponent<Velocity2D>(entity)) {
                                               Velocity2D& vel = registry.getComponent<Velocity2D>(entity);
                                               vel.vy = +100.0f;
                                           }
                                       });
    _player->bindActionCallbackOnReleased("move_down",
                                          [sendMessage](Registry& registry, system_context context, Entity entity) {
                                              sendMessage(context, "move_down", false);
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

void GameManager::update(ECS& ecs) {}