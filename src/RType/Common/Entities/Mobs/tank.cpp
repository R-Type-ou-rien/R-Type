/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Tank Spawner - Heavy enemy with high HP and player-targeting shots
*/

#include <string>
#include "all_mobs.hpp"
#include "Components/StandardComponents.hpp"
#include "ResourceConfig.hpp"
#include "../../Components/mob_component.hpp"
#include "../../Systems/animation_helper.hpp"

void TankSpawner::spawn(Registry& registry, system_context context, float x, float y, const EntityConfig& config) {
    Entity id = registry.createEntity();

    registry.addComponent<transform_component_s>(id, {x, y});
    registry.addComponent<Velocity2D>(id, {-config.speed.value(), 0.0f});
    registry.addComponent<HealthComponent>(id, 
        MobComponentFactory::createHealth(config, MobDefaults::Health::DAMAGE_FLASH_DURATION_TANK));
    registry.addComponent<TeamComponent>(id, {TeamComponent::ENEMY});
    registry.addComponent<DamageOnCollision>(id, {config.damage.value()});
    registry.addComponent<ScoreValueComponent>(id, {config.score_value.value()});

    if (config.can_shoot.value_or(false)) {
        registry.addComponent<ShooterComponent>(id, MobComponentFactory::createShooter(config));
    }

    if (config.shoot_at_player.value_or(false)) {
        registry.addComponent<BehaviorComponent>(id, 
            MobComponentFactory::createBehavior(config, MobDefaults::Behavior::FOLLOW_SPEED_TANK));
    }

    handle_t<TextureAsset> handle =
        context.texture_manager.load(config.sprite_path.value(), TextureAsset(config.sprite_path.value()));
    registry.addComponent<sprite2D_component_s>(id, MobComponentFactory::createSprite(config, handle));

    auto [num_frames, anim_speed] = MobComponentFactory::getAnimationParams(config);
    if (num_frames > 1) {
        AnimationHelper::setupHorizontalAnimation(registry, id, config, num_frames, anim_speed);
    }

    MobComponentFactory::applyScale(registry, id, config);

    registry.addComponent<BoxCollisionComponent>(id, MobComponentFactory::createEnemyCollision(config));
    registry.addComponent<TagComponent>(id, MobComponentFactory::createAITags());
    registry.addComponent<NetworkIdentity>(id, {static_cast<uint32_t>(id), 0});
}
