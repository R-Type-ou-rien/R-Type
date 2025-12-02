/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** MoveSystem.hpp
*/

#pragma once

#include "ISystem.hpp"
#include "../Components/Components.hpp"

class MoveSystem : public ISystem {
    public:
        void init(Registry& registry) override {}

        void update(Registry& registry, float dt) override {
            auto& velocities = registry.getView<Velocity2D>();
            auto& positions = registry.getView<Position2D>();
            const auto& entities = registry.getEntities<Velocity2D>();

            for (size_t i = 0; i < entities.size(); ++i) {
                EntityID entity = entities[i];

                if (registry.hasComponent<Position2D>(entity)) {
                    auto& pos = registry.getComponent<Position2D>(entity);
                    auto& vel = velocities[i];
                    pos.x += vel.vx * dt;
                    pos.y += vel.vy * dt;
                }
            }
        }
};