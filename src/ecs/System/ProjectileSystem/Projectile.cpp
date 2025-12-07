/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ProjectileSystem.cpp
*/

#include <vector>

#include "ProjectileSystem.hpp"

void ProjectileSystem::update(Registry& registry, system_context context) {
    auto& projectiles = registry.getView<Projectile>();
    const auto& entities = registry.getEntities<Projectile>();

    std::vector<Entity> toDestroy;

    for (std::size_t i = 0; i < entities.size(); ++i) {
        Entity entity = entities[i];
        auto& proj = projectiles[i];

        proj.lifetime -= context.dt;
        if (proj.lifetime <= 0.f) {
            toDestroy.push_back(entity);
        }
    }

    // On détruit à part pour éviter de casser les vecteurs pendant la boucle
    for (Entity e : toDestroy) {
        registry.destroyEntity(e);
    }
}
