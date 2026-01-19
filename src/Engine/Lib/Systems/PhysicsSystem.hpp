/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** PhysicsSystem.hpp
*/

#pragma once

#include "Components/StandardComponents.hpp"
#include "ISystem.hpp"

class PhysicsSystem : public ISystem {
   public:
    void update(Registry& registry, system_context context) override;

    static void applyMovement(TransformComponent& pos, const Velocity2D& vel, float dt) {
        pos.x += vel.vx * dt;
        pos.y += vel.vy * dt;
    }
};
