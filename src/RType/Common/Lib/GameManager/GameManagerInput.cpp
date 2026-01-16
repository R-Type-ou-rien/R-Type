#include "GameManager.hpp"
#include "ECS.hpp"
#include "InputConfig.hpp"
#include "src/RType/Common/Components/charged_shot.hpp"
#include "src/RType/Common/Components/pod_component.hpp"

void GameManager::setupMovementControls(InputManager& inputs) {
    inputs.bindAction("move_left", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Left});
    inputs.bindAction("move_right", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Right});
    inputs.bindAction("move_up", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Up});
    inputs.bindAction("move_down", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Down});

    if (_player) {
        float player_speed = _player_config.speed.value();

        _player->bindActionCallbackPressed("move_left",
                                           [player_speed](Registry& registry, system_context context, Entity entity) {
                                               if (registry.hasComponent<Velocity2D>(entity)) {
                                                   registry.getComponent<Velocity2D>(entity).vx = -player_speed;
                                               }
                                           });
        _player->bindActionCallbackOnReleased("move_left",
                                              [](Registry& registry, system_context context, Entity entity) {
                                                  if (registry.hasComponent<Velocity2D>(entity)) {
                                                      registry.getComponent<Velocity2D>(entity).vx = 0.0f;
                                                  }
                                              });

        _player->bindActionCallbackPressed("move_right",
                                           [player_speed](Registry& registry, system_context context, Entity entity) {
                                               if (registry.hasComponent<Velocity2D>(entity)) {
                                                   registry.getComponent<Velocity2D>(entity).vx = player_speed;
                                               }
                                           });
        _player->bindActionCallbackOnReleased("move_right",
                                              [](Registry& registry, system_context context, Entity entity) {
                                                  if (registry.hasComponent<Velocity2D>(entity)) {
                                                      registry.getComponent<Velocity2D>(entity).vx = 0.0f;
                                                  }
                                              });

        _player->bindActionCallbackPressed("move_up",
                                           [player_speed](Registry& registry, system_context context, Entity entity) {
                                               if (registry.hasComponent<Velocity2D>(entity)) {
                                                   registry.getComponent<Velocity2D>(entity).vy = -player_speed;
                                               }
                                           });
        _player->bindActionCallbackOnReleased("move_up", [](Registry& registry, system_context context, Entity entity) {
            if (registry.hasComponent<Velocity2D>(entity)) {
                registry.getComponent<Velocity2D>(entity).vy = 0.0f;
            }
        });

        _player->bindActionCallbackPressed("move_down",
                                           [player_speed](Registry& registry, system_context context, Entity entity) {
                                               if (registry.hasComponent<Velocity2D>(entity)) {
                                                   registry.getComponent<Velocity2D>(entity).vy = player_speed;
                                               }
                                           });
        _player->bindActionCallbackOnReleased("move_down",
                                              [](Registry& registry, system_context context, Entity entity) {
                                                  if (registry.hasComponent<Velocity2D>(entity)) {
                                                      registry.getComponent<Velocity2D>(entity).vy = 0.0f;
                                                  }
                                              });
    }
}

void GameManager::setupShootingControls(InputManager& inputs) {
    inputs.bindAction("shoot", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Space});

    if (_player) {
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
}

void GameManager::setupPodControls(InputManager& inputs) {
    inputs.bindAction("toggle_pod", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::E});

    if (_player) {
        _player->bindActionCallbackPressed("toggle_pod", [](Registry& registry, system_context context, Entity entity) {
            if (registry.hasComponent<PlayerPodComponent>(entity)) {
                auto& player_pod = registry.getComponent<PlayerPodComponent>(entity);
                if (player_pod.has_pod) {
                    player_pod.detach_requested = true;
                }
            }
        });
    }
}

void GameManager::loadInputSetting(InputManager& inputs) {
    setupMovementControls(inputs);
    setupShootingControls(inputs);
    setupPodControls(inputs);
}
