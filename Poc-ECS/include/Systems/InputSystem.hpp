/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** InputSystem.hpp
*/

#pragma once

#include <SFML/Window/Keyboard.hpp>
#include "ISystem.hpp"
#include "Components/Components.hpp"

class InputSystem : public ISystem {
    public:
        void init(Registry& registry) override {
        }

        void update(Registry& registry, float dt) override {
            const auto& entities = registry.getEntities<InputControl>();
            auto& inputs = registry.getView<InputControl>();

            for (size_t i = 0; i < entities.size(); ++i) {
                EntityID entity = entities[i];

                if (registry.hasComponent<Velocity2D>(entity)) {
                    auto& vel = registry.getComponent<Velocity2D>(entity);
                    const auto& ctrl = inputs[i];
                    vel.vx = 0.0f;
                    vel.vy = 0.0f;

                    if (sf::Keyboard::isKeyPressed(ctrl.left)) {
                        vel.vx = -ctrl.speed;
                    } else if (sf::Keyboard::isKeyPressed(ctrl.right)) {
                        vel.vx = ctrl.speed;
                    }

                    if (sf::Keyboard::isKeyPressed(ctrl.up)) {
                        vel.vy = -ctrl.speed;
                    } else if (sf::Keyboard::isKeyPressed(ctrl.down)) {
                        vel.vy = ctrl.speed;
                    }
                }
            }
        }
};