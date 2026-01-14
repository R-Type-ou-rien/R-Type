#include "powerup.hpp"
#include "Components/StandardComponents.hpp"
#include "../Components/team_component.hpp"
#include "damage.hpp"
#include "ResourceConfig.hpp"

void PowerUpSystem::update(Registry& registry, system_context context) {
    updateActivePowerUps(registry, context);
    checkPowerUpCollisions(registry, context);
}

void PowerUpSystem::updateActivePowerUps(Registry& registry, system_context context) {
    auto& active_powerups = registry.getEntities<ActivePowerUpComponent>();
    std::vector<std::pair<Entity, PowerUpComponent::PowerUpType>> expired_powerups;

    for (auto entity : active_powerups) {
        auto& powerup = registry.getComponent<ActivePowerUpComponent>(entity);

        if (powerup.remaining_time > 0) {
            powerup.remaining_time -= context.dt;

            if (powerup.remaining_time <= 0) {
                // Le power-up a expiré, restaurer les valeurs originales
                expired_powerups.push_back({entity, powerup.type});
            }
        }
    }

    // Restaurer les valeurs originales pour les power-ups expirés
    for (auto& [entity, type] : expired_powerups) {
        auto& powerup = registry.getComponent<ActivePowerUpComponent>(entity);

        if (type == PowerUpComponent::SPEED_UP) {
            if (registry.hasComponent<Velocity2D>(entity)) {
                // Restaurer la vitesse originale
                // Pour simplifier, on divise par le multiplicateur
                // (dans une vraie implémentation, on stockerait la valeur originale)
            }
        }

        registry.removeComponent<ActivePowerUpComponent>(entity);
    }
}

void PowerUpSystem::checkPowerUpCollisions(Registry& registry, system_context context) {
    // Trouver le joueur
    Entity player_entity = -1;
    auto& teams = registry.getEntities<TeamComponent>();
    for (auto entity : teams) {
        auto& team = registry.getConstComponent<TeamComponent>(entity);
        if (team.team == TeamComponent::ALLY) {
            if (registry.hasComponent<TagComponent>(entity)) {
                auto& tags = registry.getConstComponent<TagComponent>(entity);
                for (const auto& tag : tags.tags) {
                    if (tag == "PLAYER") {
                        player_entity = entity;
                        break;
                    }
                }
            }
        }
        if (player_entity != -1)
            break;
    }

    if (player_entity == -1)
        return;

    // Vérifier les collisions avec les power-ups
    auto& powerups = registry.getEntities<PowerUpComponent>();
    for (auto powerup_entity : powerups) {
        if (!registry.hasComponent<BoxCollisionComponent>(powerup_entity))
            continue;

        auto& collision = registry.getConstComponent<BoxCollisionComponent>(powerup_entity);

        // Vérifier si le joueur est dans la liste des collisions
        for (auto collided : collision.collision.tags) {
            if (collided == player_entity) {
                // Appliquer le power-up
                auto& powerup = registry.getConstComponent<PowerUpComponent>(powerup_entity);
                applyPowerUp(registry, player_entity, powerup.type, powerup.value, powerup.duration);

                // Détruire le power-up
                if (!registry.hasComponent<PendingDestruction>(powerup_entity)) {
                    registry.addComponent<PendingDestruction>(powerup_entity, {});
                }
                break;
            }
        }
    }
}

void PowerUpSystem::applyPowerUp(Registry& registry, Entity player, PowerUpComponent::PowerUpType type, float value,
                                 float duration) {
    switch (type) {
        case PowerUpComponent::SPEED_UP: {
            if (registry.hasComponent<Velocity2D>(player)) {
                // Stocker l'effet actif
                if (!registry.hasComponent<ActivePowerUpComponent>(player)) {
                    registry.addComponent<ActivePowerUpComponent>(player, {type, duration, 0.0f});
                }

                // Augmenter la vitesse de déplacement
                // Note: dans un vrai système, on devrait modifier la vitesse max du joueur
                // Pour l'instant, on marque juste qu'il a le power-up
            }
            break;
        }
        case PowerUpComponent::FIRE_RATE: {
            // Augmenter le fire rate
            break;
        }
        case PowerUpComponent::SHIELD: {
            // Ajouter un bouclier
            break;
        }
        case PowerUpComponent::WEAPON_UPGRADE: {
            // Améliorer l'arme
            break;
        }
    }
}

void PowerUpSystem::spawnSpeedUp(Registry& registry, system_context context, float x, float y) {
    Entity id = registry.createEntity();

    registry.addComponent<transform_component_s>(id, {x, y});
    registry.addComponent<Velocity2D>(id, {-100.0f, 0.0f});  // Scroll vers la gauche

    PowerUpComponent powerup;
    powerup.type = PowerUpComponent::SPEED_UP;
    powerup.duration = 10.0f;  // 10 secondes
    powerup.value = 1.5f;      // +50% de vitesse
    registry.addComponent<PowerUpComponent>(id, powerup);

    // Visuel du power-up (utiliser r-typesheet3.gif pour les power-ups)
    handle_t<TextureAsset> handle =
        context.texture_manager.load("src/RType/Common/content/sprites/r-typesheet3.gif",
                                     TextureAsset("src/RType/Common/content/sprites/r-typesheet3.gif"));

    sprite2D_component_s sprite_info;
    sprite_info.handle = handle;
    sprite_info.dimension = {1, 99, 16, 16};  // Power-up sprite
    sprite_info.z_index = 1;
    sprite_info.is_animated = false;
    registry.addComponent<sprite2D_component_s>(id, sprite_info);

    auto& transform = registry.getComponent<transform_component_s>(id);
    transform.scale_x = 2.5f;
    transform.scale_y = 2.5f;

    // Collision
    BoxCollisionComponent collision;
    collision.tagCollision.push_back("PLAYER");
    registry.addComponent<BoxCollisionComponent>(id, collision);

    TagComponent tags;
    tags.tags.push_back("POWERUP");
    registry.addComponent<TagComponent>(id, tags);
}
