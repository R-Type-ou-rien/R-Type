/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** WallCollisionSystem - Handles player collision with indestructible walls
*/

#pragma once

#include "ISystem.hpp"
#include "registry.hpp"

class WallCollisionSystem : public ISystem {
   public:
    WallCollisionSystem() = default;
    ~WallCollisionSystem() = default;

    void update(Registry& registry, system_context context) override;

   private:
    void handlePlayerWallCollision(Registry& registry, Entity player, Entity wall);
    void handleProjectileWallCollision(Registry& registry, Entity projectile, Entity wall);
};
