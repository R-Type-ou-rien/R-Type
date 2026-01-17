/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Obstacle Spawner - Static or slow-moving obstacles in the level
*/

#include "all_mobs.hpp"
#include "Components/StandardComponents.hpp"
#include "ResourceConfig.hpp"
#include "../../Components/mob_component.hpp"
#include <string>

Entity ObstacleSpawner::spawn(Registry& registry, system_context context, float x, float y,
                              const EntityConfig& config) {
    Entity id = registry.createEntity();
    const int hp = config.hp.value_or(MobDefaults::Obstacle::HP);
    const int damage = config.damage.value_or(MobDefaults::Obstacle::DAMAGE);

    registry.addComponent<transform_component_s>(id, {x, y});
    registry.addComponent<Velocity2D>(id, {MobDefaults::Obstacle::VELOCITY_X, 0.0f});
    registry.addComponent<HealthComponent>(id, {hp, hp, 0.0f, 0.0f});
    registry.addComponent<DamageOnCollision>(id, {damage});

    const std::string sprite_path = config.sprite_path.value_or(MobDefaults::Obstacle::SPRITE_PATH);
    handle_t<TextureAsset> handle = context.texture_manager.load(sprite_path, TextureAsset(sprite_path));

    // sprite2D_component_s sprite_info;
    // sprite_info.handle = handle;
    // sprite_info.dimension = {
    //     static_cast<float>(config.sprite_x.value_or(static_cast<int>(MobDefaults::Obstacle::SPRITE_X))),
    //     static_cast<float>(config.sprite_y.value_or(static_cast<int>(MobDefaults::Obstacle::SPRITE_Y))),
    //     static_cast<float>(config.sprite_w.value_or(static_cast<int>(MobDefaults::Obstacle::SPRITE_W))),
    //     static_cast<float>(config.sprite_h.value_or(static_cast<int>(MobDefaults::Obstacle::SPRITE_H)))
    // };
    // sprite_info.z_index = MobDefaults::Sprite::Z_INDEX;
    // registry.addComponent<sprite2D_component_s>(id, sprite_info);

    AnimatedSprite2D animation;
    AnimationClip clip;

    clip.handle = handle;
    clip.frames.emplace_back(
        static_cast<float>(config.sprite_x.value_or(static_cast<int>(MobDefaults::Obstacle::SPRITE_X))),
        static_cast<float>(config.sprite_y.value_or(static_cast<int>(MobDefaults::Obstacle::SPRITE_Y))),
        static_cast<float>(config.sprite_w.value_or(static_cast<int>(MobDefaults::Obstacle::SPRITE_W))),
        static_cast<float>(config.sprite_h.value_or(static_cast<int>(MobDefaults::Obstacle::SPRITE_H))));
    animation.animations.emplace("idle", clip);
    animation.currentAnimation = "idle";
    animation.layer = static_cast<RenderLayer>(MobDefaults::Sprite::Z_INDEX);
    registry.addComponent<AnimatedSprite2D>(id, animation);

    auto& transform = registry.getComponent<transform_component_s>(id);
    const float scale = config.scale.value_or(MobDefaults::Obstacle::SCALE);

    transform.scale_x = scale;
    transform.scale_y = scale;
    registry.addComponent<BoxCollisionComponent>(id, MobComponentFactory::createEnemyCollision(config));
    registry.addComponent<TagComponent>(id, MobComponentFactory::createObstacleTags());
    registry.addComponent<NetworkIdentity>(id, {static_cast<uint32_t>(id), 0});

    return id;
}
