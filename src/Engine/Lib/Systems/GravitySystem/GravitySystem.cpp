/*
** EPITECH PROJECT, 2025
** Smash
** File description:
** GravitySystem.cpp
*/

#include "GravitySystem.hpp"
#include "../../Components/GravityComponent.hpp"
#include "../../Components/StructDatas/Vector2D.hpp"
#include "../../Components/StandardComponents.hpp"
#include "../../Components/GroundComponent.hpp"

void GravitySystem::update(Registry& registry, system_context context) {
    auto& gravityPool = registry.getEntities<GravityComponent>();

    for (Entity entity : gravityPool) {
        if (!registry.hasComponent<Velocity2D>(entity))
            continue;

        auto& gravity = registry.getComponent<GravityComponent>(entity);
        auto& velocity = registry.getComponent<Velocity2D>(entity);

        if (!gravity.grounded) {
            gravity.vectorY += gravity.force * context.dt;
        } else {
            gravity.vectorY = 0.0f;
        }

        checkGrounded(registry, entity, context);
    }
}

void GravitySystem::checkGrounded(Registry& registry, Entity entity, system_context context) {
    auto& gravity = registry.getComponent<GravityComponent>(entity);
    auto& velocity = registry.getComponent<Velocity2D>(entity);
    auto& transform = registry.getComponent<TransformComponent>(entity);
    auto& ground = registry.getEntities<GroundComponent>();

    for (Entity groundEntity : ground) {
        auto& groundComp = registry.getComponent<GroundComponent>(groundEntity);

        if (transform.y + (velocity.vy * context.dt) + gravity.vectorY >= groundComp.rect.y &&
            transform.y + (velocity.vy * context.dt) + gravity.vectorY <= groundComp.rect.y + groundComp.rect.height &&
            transform.x + (velocity.vx * context.dt) >= groundComp.rect.x &&
            transform.x + (velocity.vx * context.dt) <= groundComp.rect.x + groundComp.rect.width) {
            gravity.grounded = true;
            transform.y = groundComp.rect.y;
            gravity.vectorY = 0.0f;
            return;
        } else {
            gravity.grounded = false;
        }
    }
}
