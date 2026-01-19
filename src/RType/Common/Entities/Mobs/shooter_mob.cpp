/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Shooter Spawner - Enemy that actively shoots at the player
*/

#include <string>
#include "all_mobs.hpp"
#include "Components/StandardComponents.hpp"
#include "ResourceConfig.hpp"
#include "../../Components/mob_component.hpp"
#include "../../Systems/animation_helper.hpp"

Entity ShooterSpawner::spawn(Registry& registry, system_context context, float x, float y, const EntityConfig& config) {
    Entity id = registry.createEntity();

    registry.addComponent<TransformComponent>(id, {x, y});
    registry.addComponent<Velocity2D>(id, {-config.speed.value(), 0.0f});
    registry.addComponent<HealthComponent>(id, MobComponentFactory::createHealth(config));
    registry.addComponent<TeamComponent>(id, {TeamComponent::ENEMY});
    registry.addComponent<DamageOnCollision>(id, {config.damage.value()});
    registry.addComponent<ScoreValueComponent>(id, {config.score_value.value()});
    registry.addComponent<ShooterComponent>(id, MobComponentFactory::createShooter(config));

    BehaviorComponent behavior = MobComponentFactory::createBehavior(config);
    behavior.shoot_at_player = config.shoot_at_player.value_or(true);
    registry.addComponent<BehaviorComponent>(id, behavior);

    handle_t<TextureAsset> handle =
        context.texture_manager.load(config.sprite_path.value(), TextureAsset(config.sprite_path.value()));
    // registry.addComponent<Sprite2DComponent>(id, MobComponentFactory::createSprite(config, handle));
    registry.addComponent<AnimatedSprite2D>(id, MobComponentFactory::createAnimatedSprite(config, handle));

    auto [num_frames, anim_speed] = MobComponentFactory::getAnimationParams(config);
    if (num_frames > 1) {
        AnimationHelper::setupHorizontalAnimation(registry, id, config, num_frames, anim_speed);
    }

    MobComponentFactory::applyScale(registry, id, config);

    registry.addComponent<BoxCollisionComponent>(id, MobComponentFactory::createEnemyCollision(config));
    registry.addComponent<TagComponent>(id, MobComponentFactory::createAITags());
    registry.addComponent<NetworkIdentity>(id, {static_cast<uint32_t>(id), 0});

    return id;
}
