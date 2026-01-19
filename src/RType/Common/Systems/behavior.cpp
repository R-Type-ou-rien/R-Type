#include "behavior.hpp"
#include "Components/StandardComponents.hpp"
#include "../Components/team_component.hpp"
#include "shooter.hpp"
#include "health.hpp"
#include "src/Engine/Lib/Systems/PlayerBoundsSystem.hpp"
#include <cmath>

void BehaviorSystem::update(Registry& registry, system_context context) {
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

    auto& behaviors = registry.getEntities<BehaviorComponent>();
    for (auto entity : behaviors) {
        auto& behavior = registry.getComponent<BehaviorComponent>(entity);

        if (behavior.follow_player) {
            updateFollowPlayer(registry, context, entity, player_entity);
        }

        if (behavior.shoot_at_player) {
            updateShootAtPlayer(registry, entity, player_entity);
        }
    }
}

void BehaviorSystem::updateFollowPlayer(Registry& registry, system_context context, Entity enemy, Entity player) {
    if (!registry.hasComponent<TransformComponent>(enemy) || !registry.hasComponent<TransformComponent>(player)) {
        return;
    }

    auto& enemy_transform = registry.getComponent<TransformComponent>(enemy);
    auto& player_transform = registry.getConstComponent<TransformComponent>(player);
    auto& behavior = registry.getConstComponent<BehaviorComponent>(enemy);

    float dx = player_transform.x - enemy_transform.x;
    float dy = player_transform.y - enemy_transform.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance > 5.0f) {
        if (registry.hasComponent<Velocity2D>(enemy)) {
            auto& vel = registry.getComponent<Velocity2D>(enemy);
            vel.vx = (dx / distance) * behavior.follow_speed;
            vel.vy = (dy / distance) * behavior.follow_speed;
        }
    }
}

void BehaviorSystem::updateShootAtPlayer(Registry& registry, Entity enemy, Entity player) {}

void BoundsSystem::update(Registry& registry, system_context context) {
#if defined(CLIENT_BUILD)
    const float windowWidth = static_cast<float>(context.window.getSize().x);
    const float windowHeight = static_cast<float>(context.window.getSize().y);
#else
    const float windowWidth = 1920.0f;
    const float windowHeight = 1080.0f;
#endif

    float min_x = 0.0f;
    float min_y = 0.0f;

    auto& bounds_entities = registry.getEntities<WorldBoundsComponent>();
    if (!bounds_entities.empty()) {
        auto& bounds = registry.getConstComponent<WorldBoundsComponent>(bounds_entities[0]);
        min_x = bounds.min_x;
        min_y = bounds.min_y;
    }

    auto& players = registry.getEntities<TeamComponent>();
    for (auto entity : players) {
        auto& team = registry.getConstComponent<TeamComponent>(entity);
        if (team.team != TeamComponent::ALLY)
            continue;

        if (!registry.hasComponent<TagComponent>(entity))
            continue;
        auto& tags = registry.getConstComponent<TagComponent>(entity);

        bool is_player = false;
        for (const auto& tag : tags.tags) {
            if (tag == "PLAYER") {
                is_player = true;
                break;
            }
        }

        if (!is_player)
            continue;

        if (registry.hasComponent<TransformComponent>(entity)) {
            auto& transform = registry.getComponent<TransformComponent>(entity);

            float sprite_w = 33.0f;
            float sprite_h = 17.0f;
            float scale_x = transform.scale_x;
            float scale_y = transform.scale_y;

            // if (registry.hasComponent<Sprite2DComponent>(entity)) {
            //     auto& sprite = registry.getConstComponent<Sprite2DComponent>(entity);
            //     if (sprite.is_animated && !sprite.frames.empty()) {
            //         const auto& frame = sprite.frames[sprite.current_animation_frame];
            //         sprite_w = frame.width;
            //         sprite_h = frame.height;
            //     } else if (sprite.dimension.width > 0 && sprite.dimension.height > 0) {
            //         sprite_w = sprite.dimension.width;
            //         sprite_h = sprite.dimension.height;
            //     }
            // }

            if (registry.hasComponent<AnimatedSprite2D>(entity)) {
                auto& animation = registry.getConstComponent<AnimatedSprite2D>(entity);
                if (!animation.animations.at(animation.currentAnimation).frames.empty()) {
                    const auto& frame = animation.animations.at(animation.currentAnimation).frames;
                    sprite_w = animation.animations.at(animation.currentAnimation)
                                   .frames.at(animation.currentFrameIndex)
                                   .width;
                    sprite_h = animation.animations.at(animation.currentAnimation)
                                   .frames.at(animation.currentFrameIndex)
                                   .height;
                }
            } else if (registry.hasComponent<Sprite2D>(entity)) {
                auto& sprite = registry.getConstComponent<Sprite2D>(entity);

                sprite_w = sprite.rect.width;
                sprite_h = sprite.rect.height;
            }

            float actual_width = sprite_w * std::abs(scale_x);
            float actual_height = sprite_h * std::abs(scale_y);

            if (transform.x < min_x)
                transform.x = min_x;
            if (transform.x > windowWidth - actual_width)
                transform.x = windowWidth - actual_width;
            if (transform.y < min_y)
                transform.y = min_y;
            if (transform.y > windowHeight - actual_height)
                transform.y = windowHeight - actual_height;
        }
    }
}
