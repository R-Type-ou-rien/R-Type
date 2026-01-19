#include "CollisionSystem.hpp"
#include <iostream>
#include <iterator>
#include <ostream>
#include <vector>
#include <utility>
#include <algorithm>
#include "Components/StandardComponents.hpp"
#include "../Components/LobbyIdComponent.hpp"
#include "../Utils/LobbyUtils.hpp"

void BoxCollision::update(Registry& registry, system_context context) {
    auto& entities = registry.getEntities<BoxCollisionComponent>();

    for (auto entity : entities) {
        auto& col = registry.getComponent<BoxCollisionComponent>(entity);
        col.collision.tags.clear();
    }

    for (auto entity_a : entities) {
        if (!registry.hasComponent<TransformComponent>(entity_a))
            continue;
        if (!registry.hasComponent<TagComponent>(entity_a))
            continue;

        uint32_t lobby_a = engine::utils::getLobbyId(registry, entity_a);

        auto& transform_a = registry.getConstComponent<TransformComponent>(entity_a);
        auto& collision_comp = registry.getComponent<BoxCollisionComponent>(entity_a);
        for (auto entity_b : entities) {
            if (entity_a == entity_b)
                continue;

            // Skip collision if entities are from different lobbies
            uint32_t lobby_b = engine::utils::getLobbyId(registry, entity_b);
            if (lobby_a != lobby_b && lobby_a != 0 && lobby_b != 0)
                continue;

            if (!registry.hasComponent<TagComponent>(entity_b))
                continue;
            if (!hasTagToCollide(collision_comp, registry.getConstComponent<TagComponent>(entity_b)))
                continue;
            if (!registry.hasComponent<TransformComponent>(entity_b))
                continue;
            // if (!registry.hasComponent<Sprite2DComponent>(entity_a))
            //     continue;
            // if (!registry.hasComponent<Sprite2DComponent>(entity_b))
            //     continue;
            float sprite_a_width;
            float sprite_a_height;
            float sprite_b_width;
            float sprite_b_height;
            if (registry.hasComponent<AnimatedSprite2D>(entity_a)) {
                auto& sprite_a = registry.getConstComponent<AnimatedSprite2D>(entity_a);
                sprite_a_width =
                    sprite_a.animations.at(sprite_a.currentAnimation).frames.at(sprite_a.currentFrameIndex).width;
                sprite_a_height =
                    sprite_a.animations.at(sprite_a.currentAnimation).frames.at(sprite_a.currentFrameIndex).height;
            } else if (registry.hasComponent<Sprite2D>(entity_a)) {
                auto& sprite_a = registry.getConstComponent<Sprite2D>(entity_a);
                sprite_a_width = sprite_a.rect.width;
                sprite_a_height = sprite_a.rect.height;
            } else
                continue;

            if (registry.hasComponent<AnimatedSprite2D>(entity_b)) {
                auto& sprite_b = registry.getConstComponent<AnimatedSprite2D>(entity_b);
                sprite_b_width =
                    sprite_b.animations.at(sprite_b.currentAnimation).frames.at(sprite_b.currentFrameIndex).width;
                sprite_b_height =
                    sprite_b.animations.at(sprite_b.currentAnimation).frames.at(sprite_b.currentFrameIndex).height;
            } else if (registry.hasComponent<Sprite2D>(entity_b)) {
                auto& sprite_b = registry.getConstComponent<Sprite2D>(entity_b);
                sprite_b_width = sprite_b.rect.width;
                sprite_b_height = sprite_b.rect.height;
            } else
                continue;
            auto& transform_b = registry.getConstComponent<TransformComponent>(entity_b);

            Velocity2D vel_a = {0, 0};
            if (registry.hasComponent<Velocity2D>(entity_a))
                vel_a = registry.getConstComponent<Velocity2D>(entity_a);

            Velocity2D vel_b = {0, 0};
            if (registry.hasComponent<Velocity2D>(entity_b))
                vel_b = registry.getConstComponent<Velocity2D>(entity_b);

            // if (checkSize(transform_a, transform_b, {sprite_a.dimension.width, sprite_a.dimension.height},
            //               {sprite_b.dimension.width, sprite_b.dimension.height}, vel_a, vel_b, context.dt)) {
            //     collision_comp.collision.tags.push_back(entity_b);
            // }

            if (checkSize(transform_a, transform_b, {sprite_a_width, sprite_a_height},
                          {sprite_b_width, sprite_b_height}, vel_a, vel_b, context.dt)) {
                collision_comp.collision.tags.push_back(entity_b);
            }
        }
        if (collision_comp.callbackOnCollide && !collision_comp.collision.tags.empty())
            collision_comp.callbackOnCollide(registry, context, entity_a);
    }
}

bool BoxCollision::checkSize(const TransformComponent a, const TransformComponent b,
                             std::pair<float, float> size_a, std::pair<float, float> size_b, Velocity2D vel_a,
                             Velocity2D vel_b, float dt) {
    double width_a = size_a.first * a.scale_x;
    double height_a = size_a.second * a.scale_y;
    double width_b = size_b.first * b.scale_x;
    double height_b = size_b.second * b.scale_y;
    double a_min_x = std::min(a.x, a.x + vel_a.vx * dt);
    double a_max_x = std::max(a.x + width_a, a.x + width_a + vel_a.vx * dt);
    double a_min_y = std::min(a.y, a.y + vel_a.vy * dt);
    double a_max_y = std::max(a.y + height_a, a.y + height_a + vel_a.vy * dt);
    double b_min_x = std::min(b.x, b.x + vel_b.vx * dt);
    double b_max_x = std::max(b.x + width_b, b.x + width_b + vel_b.vx * dt);
    double b_min_y = std::min(b.y, b.y + vel_b.vy * dt);
    double b_max_y = std::max(b.y + height_b, b.y + height_b + vel_b.vy * dt);
    bool collision_x = a_min_x < b_max_x && a_max_x > b_min_x;
    bool collision_y = a_min_y < b_max_y && a_max_y > b_min_y;

    return collision_x && collision_y;
}

bool BoxCollision::hasTagToCollide(BoxCollisionComponent entity_a, const TagComponent& entity_b) {
    if (entity_a.tagCollision.empty()) {
        return false;
    }
    for (auto tag_to_collide : entity_a.tagCollision) {
        for (auto tag : entity_b.tags) {
            if (tag_to_collide == tag)
                return true;
        }
    }
    return false;
}
