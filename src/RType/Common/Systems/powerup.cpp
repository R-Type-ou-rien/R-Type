#include <vector>
#include "powerup.hpp"
#include "Components/StandardComponents.hpp"
#include "../Components/team_component.hpp"
#include "damage.hpp"
#include "ResourceConfig.hpp"
#include "../../../../Engine/Lib/Components/LobbyIdComponent.hpp"

PowerUpSystem::PowerUpSystem() {
    initialize_power_up_maps();
}

void PowerUpSystem::initialize_power_up_maps() {
    apply_power_up[PowerUpComponent::SPEED_UP] = [](Registry& registry, Entity player, float value, float duration) {
        if (registry.hasComponent<Velocity2D>(player)) {
            if (!registry.hasComponent<ActivePowerUpComponent>(player)) {
                registry.addComponent<ActivePowerUpComponent>(player, {PowerUpComponent::SPEED_UP, duration, 0.0f});
            }
        }
    };

    deactivate_power_up[PowerUpComponent::SPEED_UP] = [](Registry& registry, Entity player) {};
}

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
                expired_powerups.push_back({entity, powerup.type});
            }
        }
    }

    for (auto& [entity, type] : expired_powerups) {
        auto it = deactivate_power_up.find(type);
        if (it != deactivate_power_up.end()) {
            it->second(registry, entity);
        }

        registry.removeComponent<ActivePowerUpComponent>(entity);
    }
}

void PowerUpSystem::checkPowerUpCollisions(Registry& registry, system_context context) {
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

    auto& powerups = registry.getEntities<PowerUpComponent>();
    for (auto powerup_entity : powerups) {
        if (!registry.hasComponent<BoxCollisionComponent>(powerup_entity))
            continue;

        auto& collision = registry.getConstComponent<BoxCollisionComponent>(powerup_entity);

        for (auto collided : collision.collision.tags) {
            if (collided == player_entity) {
                auto& powerup = registry.getConstComponent<PowerUpComponent>(powerup_entity);
                applyPowerUp(registry, player_entity, powerup.type, powerup.value, powerup.duration);

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
    auto it = apply_power_up.find(type);
    if (it != apply_power_up.end()) {
        it->second(registry, player, value, duration);
    }
}

void PowerUpSystem::spawnSpeedUp(Registry& registry, system_context context, float x, float y, uint32_t lobbyId) {
    Entity id = registry.createEntity();

    registry.addComponent<transform_component_s>(id, {x, y});
    registry.addComponent<Velocity2D>(id, {-100.0f, 0.0f});

    PowerUpComponent powerup;
    powerup.type = PowerUpComponent::SPEED_UP;
    powerup.duration = 10.0f;
    powerup.value = 1.5f;
    registry.addComponent<PowerUpComponent>(id, powerup);

    handle_t<TextureAsset> handle =
        context.texture_manager.load("src/RType/Common/content/sprites/r-typesheet3.gif",
                                     TextureAsset("src/RType/Common/content/sprites/r-typesheet3.gif"));

    // sprite2D_component_s sprite_info;
    // sprite_info.handle = handle;
    // sprite_info.dimension = {1, 99, 16, 16};  // Power-up sprite
    // sprite_info.z_index = 1;
    // sprite_info.is_animated = false;
    // registry.addComponent<sprite2D_component_s>(id, sprite_info);

    AnimatedSprite2D animation;
    AnimationClip clip;

    clip.handle = handle;
    clip.frames.emplace_back(1, 99, 16, 16);  // Power-up sprite
    animation.animations.emplace("idle", clip);
    animation.currentAnimation = "idle";
    registry.addComponent<AnimatedSprite2D>(id, animation);

    auto& transform = registry.getComponent<transform_component_s>(id);
    transform.scale_x = 2.5f;
    transform.scale_y = 2.5f;

    BoxCollisionComponent collision;
    collision.tagCollision.push_back("PLAYER");
    registry.addComponent<BoxCollisionComponent>(id, collision);

    TagComponent tags;
    tags.tags.push_back("POWERUP");
    registry.addComponent<TagComponent>(id, tags);

    // Add NetworkIdentity
    registry.addComponent<NetworkIdentity>(id, {static_cast<uint32_t>(id), 0});

    // Add Lobby Id
    if (lobbyId != 0) {
        registry.addComponent<LobbyIdComponent>(id, {lobbyId});
    }
}
