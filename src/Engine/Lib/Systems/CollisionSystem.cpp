#include "CollisionSystem.hpp"
#include <iostream>
#include <iterator>
#include <ostream>
#include "registry.hpp"
#include "Components/StandardComponents.hpp"

void BoxCollision::update(Registry& registry, system_context context) {
    auto& entities = registry.getEntities<BoxCollisionComponent>();

    for (auto entity : entities) {
        auto& col = registry.getComponent<BoxCollisionComponent>(entity);
        col.collision.tags.clear();
    }

    for (auto entity_a : entities) {
        if (!registry.hasComponent<transform_component_s>(entity_a))  
            continue;
        if (!registry.hasComponent<TagComponent>(entity_a))
            continue;
        auto& transform_a = registry.getComponent<transform_component_s>(entity_a);
        auto& collision_comp = registry.getComponent<BoxCollisionComponent>(entity_a);
        for (auto entity_b : entities) {
            if (entity_a == entity_b)  
                continue;
            if (!registry.hasComponent<TagComponent>(entity_b))  
                continue;
            if (!hasTagToCollide(collision_comp, registry.getComponent<TagComponent>(entity_b)))
                continue;
            if (!registry.hasComponent<transform_component_s>(entity_b))  
                continue;
            if (!registry.hasComponent<sprite2D_component_s>(entity_a))
                continue;
            if (!registry.hasComponent<sprite2D_component_s>(entity_b))
                continue;
            auto& transform_b = registry.getComponent<transform_component_s>(entity_b);
            auto& sprite_a = registry.getComponent<sprite2D_component_s>(entity_a);
            auto& sprite_b = registry.getComponent<sprite2D_component_s>(entity_b);
            if (checkSize(transform_a, transform_b, {sprite_a.dimension.width, sprite_a.dimension.height},
                          {sprite_b.dimension.width, sprite_b.dimension.height})) {
                collision_comp.collision.tags.push_back(entity_b);
            }
        }
        if (collision_comp.callbackOnCollide && !collision_comp.collision.tags.empty()) {
            std::cout << "[Server] Collision detected for Entity " << entity_a << " with "
                      << collision_comp.collision.tags.size() << " targets." << std::endl;
            collision_comp.callbackOnCollide(registry, context, entity_a);
        }
    }
}

bool BoxCollision::checkSize(const transform_component_s a, const transform_component_s b,
                             std::pair<float, float> size_a, std::pair<float, float> size_b) {
    double width_a = size_a.first * a.scale_x;
    double height_a = size_a.second * a.scale_y;
    double width_b = size_b.first * b.scale_x;
    double height_b = size_b.second * b.scale_y;
    double dist_x = std::abs(a.x - b.x);
    double dist_y = std::abs(a.y - b.y);
    bool colision_x = dist_x < (width_a / 2.0) + (width_b / 2.0);
    bool colision_y = dist_y < (height_a / 2.0) + (height_b / 2.0);
    return colision_x && colision_y;
}

bool BoxCollision::hasTagToCollide(BoxCollisionComponent entity_a, TagComponent entity_b) {
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
