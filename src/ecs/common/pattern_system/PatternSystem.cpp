/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** PatternSystem
*/

#include "PatternSystem.hpp"

#include <cmath>

void PatternSystem::update(Registry& registry, system_context context) {
    const auto& entities = registry.getEntities<PatternComponent>();
    float dt = context.dt;
    float tolerance = 5.0f;

    for (auto entity : entities) {
        if (!registry.hasComponent<transform_component_s>(entity))
            continue;

        auto& transform = registry.getComponent<transform_component_s>(entity);
        auto& path = registry.getComponent<PatternComponent>(entity);

        if (!path.is_active || path.waypoints.empty())
            continue;
        if (path.current_index >= path.waypoints.size())
            continue;

        sf::Vector2f target = path.waypoints[path.current_index];
        float dx = target.x - transform.x;
        float dy = target.y - transform.y;
        float distance = std::sqrt(dx * dx + dy * dy);

        if (distance <= tolerance) {
            path.current_index++;

            if (path.current_index >= path.waypoints.size()) {
                if (path.loop) {
                    path.current_index = 0;
                } else {
                    path.is_active = false;
                }
            }
        } else {
            if (distance != 0) {
                transform.x += (dx / distance) * path.speed * dt;
                transform.y += (dy / distance) * path.speed * dt;
            }
        }
    }
}
