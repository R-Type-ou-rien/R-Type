#include "../box_collision/box_collision.hpp"

#include "../../common/Components/Components.hpp"
#include "ecs/common/ISystem.hpp"

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
            if (checkSize(transform_a, transform_b, sprite_a.dimension.size, sprite_b.dimension.size)) {
                auto& box_collision_a = registry.getComponent<BoxCollisionComponent>(entity_a);
                box_collision_a.collision.tags.push_back(entity_b);
            }
        }
    }
}

bool BoxCollision::checkSize(const transform_component_s& a, const transform_component_s& b, sf::Vector2<int> size_a,
                             sf::Vector2<int> size_b) {
    double width_a = size_a.x * a.scale.x;
    double height_a = size_a.y * a.scale.y;
    double width_b = size_b.x * b.scale.x;
    double height_b = size_b.y * b.scale.y;
    double dist_x = std::abs(a.x - b.x);
    double dist_y = std::abs(a.y - b.y);
    bool colision_x = dist_x < (width_a / 2.0) + (width_b / 2.0);
    bool colision_y = dist_y < (height_a / 2.0) + (height_b / 2.0);
    return colision_x && colision_y;
}
