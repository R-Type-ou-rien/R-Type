#include "projectile_cleanup.hpp"
#include "Components/StandardComponents.hpp"
#include "shooter.hpp"
#include <vector>

void ProjectileCleanupSystem::update(Registry& registry, system_context context) {
    const float WORLD_WIDTH = 1920.0f;
    const float WORLD_HEIGHT = 1080.0f;
    const float MARGIN = 200.0f; // Marge pour détruire hors écran

    auto& projectiles = registry.getEntities<ProjectileComponent>();
    std::vector<Entity> to_destroy;

    for (auto entity : projectiles) {
        if (registry.hasComponent<transform_component_s>(entity)) {
            auto& pos = registry.getConstComponent<transform_component_s>(entity);
            
            // Détruire les projectiles hors écran pour réduire le lag
            if (pos.x < -MARGIN || pos.x > WORLD_WIDTH + MARGIN ||
                pos.y < -MARGIN || pos.y > WORLD_HEIGHT + MARGIN) {
                to_destroy.push_back(entity);
            }
        }
    }

    // Détruire les projectiles hors écran
    for (auto entity : to_destroy) {
        registry.destroyEntity(entity);
    }
}
