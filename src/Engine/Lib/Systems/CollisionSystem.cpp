#include "CollisionSystem.hpp"

void BoxCollision::update(Registry& registry, system_context context) {
    auto& entities = registry.getEntities<BoxCollisionComponent>();

    for (auto entity : entities) {
        registry.getComponent<BoxCollisionComponent>(entity).collision.tags.clear();  // clear the previeous collisions
    }

    for (auto entity_a : entities) {
        if (!registry.hasComponent<transform_component_s>(entity_a))  // check if the entity has a transform
            continue;
        auto& transform_a = registry.getComponent<transform_component_s>(entity_a);
        for (auto entity_b : entities) {
            if (entity_a == entity_b)  // check if its the same entity
                continue;
            if (!registry.hasComponent<transform_component_s>(entity_b))  // check if the entity has a transform
                continue;
            if (!registry.hasComponent<sprite2D_component_s>(entity_a))
                continue;
            if (!registry.hasComponent<sprite2D_component_s>(entity_b))
                continue;
            auto& transform_b = registry.getComponent<transform_component_s>(entity_b);
            auto& sprite_a = registry.getComponent<sprite2D_component_s>(entity_a);
            auto& sprite_b = registry.getComponent<sprite2D_component_s>(entity_b);
            if (checkSize(transform_a, transform_b, {sprite_a.dimension.height, sprite_a.dimension.width},
                          {sprite_b.dimension.height, sprite_b.dimension.width})) {
                auto& box_collision_a = registry.getComponent<BoxCollisionComponent>(entity_a);
                box_collision_a.collision.tags.push_back(entity_b);
            }
        }
    }
}

bool BoxCollision::checkSize(const transform_component_s& a, const transform_component_s& b,
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
