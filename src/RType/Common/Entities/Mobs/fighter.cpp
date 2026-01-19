/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Fighter Spawner - Enemy with sinusoidal movement pattern and shooting capability
*/

#include <string>
#include "all_mobs.hpp"
#include "Components/StandardComponents.hpp"
#include "ResourceConfig.hpp"
#include "../../Components/mob_component.hpp"
#include "../../Systems/animation_helper.hpp"

Entity FighterSpawner::spawn(Registry& registry, system_context context, float x, float y, const EntityConfig& config) {
    Entity id = registry.createEntity();

    registry.addComponent<TransformComponent>(id, {x, y});
    registry.addComponent<PatternComponent>(id, MobComponentFactory::createPattern(config));
    registry.addComponent<HealthComponent>(id, MobComponentFactory::createHealth(config));
    registry.addComponent<TeamComponent>(id, {TeamComponent::ENEMY});
    registry.addComponent<DamageOnCollision>(id, {config.damage.value()});
    registry.addComponent<ScoreValueComponent>(id, {config.score_value.value()});

    if (config.can_shoot.value_or(false)) {
        registry.addComponent<ShooterComponent>(id, MobComponentFactory::createShooter(config));
    }

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
