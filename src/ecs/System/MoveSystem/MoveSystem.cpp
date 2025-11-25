/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** MoveSystem.cpp
*/

#include "MoveSystem.hpp"

void MoveSystem::update(Registry& registry, float dt) {
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