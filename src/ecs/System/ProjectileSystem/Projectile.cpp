/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ProjectileSystem.cpp
*/

#include "ProjectileSystem.hpp"

void ProjectileSystem::update(Registry& registry, float dt)
{
    auto& projectiles = registry.getView<Projectile>();
    const auto& entities = registry.getEntities<Projectile>();

    std::vector<EntityID> toDestroy;

    for (std::size_t i = 0; i < entities.size(); ++i) {
        EntityID entity = entities[i];
        auto& proj = projectiles[i];

        proj.lifetime -= dt;
        if (proj.lifetime <= 0.f) {
            toDestroy.push_back(entity);
        }
    }

    // On détruit à part pour éviter de casser les vecteurs pendant la boucle
    for (EntityID e : toDestroy) {
        registry.destroyEntity(e);
    }
}
