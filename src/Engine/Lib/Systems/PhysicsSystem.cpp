/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** PhysicsSystem.cpp
*/

#include "PhysicsSystem.hpp"

void PhysicsSystem::update(Registry& registry, system_context context) {
    auto& velocities = registry.getView<Velocity2D>();
    auto& positions = registry.getView<sprite2D_component_s>();
    const auto& entities = registry.getEntities<Velocity2D>();

    for (size_t i = 0; i < entities.size(); ++i) {
        Entity entity = entities[i];

        if (registry.hasComponent<sprite2D_component_s>(entity)) {
            auto& pos = registry.getComponent<sprite2D_component_s>(entity);
            auto& vel = velocities[i];
            pos.dimension.x += vel.vx * context.dt;
            pos.dimension.y += vel.vy * context.dt;
        }
    }
}
