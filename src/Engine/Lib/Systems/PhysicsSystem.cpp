/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** PhysicsSystem.cpp
*/

#include "PhysicsSystem.hpp"
#include "Components/StandardComponents.hpp"
#include "Components/GravityComponent.hpp"

void PhysicsSystem::update(Registry& registry, system_context context) {
    auto& velocities = registry.getView<Velocity2D>();
    auto& positions = registry.getView<TransformComponent>();
    const auto& entities = registry.getEntities<Velocity2D>();

    for (size_t i = 0; i < entities.size(); ++i) {
        Entity entity = entities[i];

        if (registry.hasComponent<TransformComponent>(entity)) {
            auto& pos = registry.getComponent<TransformComponent>(entity);
            auto& vel = velocities[i];
            if (registry.hasComponent<GravityComponent>(entity)) {
                auto& gravity = registry.getComponent<GravityComponent>(entity);
                pos.y += gravity.vectorY;
            }
            pos.x += vel.vx * context.dt;
            pos.y += vel.vy * context.dt;
        }
    }
}
