/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ScrollSystem.cpp
*/

#include "ScrollSystem.hpp"

void ScrollSystem::update(Registry& registry, system_context context) {
    auto& scrolls = registry.getView<Scroll>();
    const auto& entities = registry.getEntities<Scroll>();

    for (size_t i = 0; i < entities.size(); ++i) {
        Entity entity = entities[i];

        if (registry.hasComponent<TransformComponent>(entity)) {
            auto& scroll = scrolls[i];
            if (scroll.is_paused)
                continue;

            auto& transform = registry.getComponent<TransformComponent>(entity);
            transform.x += scroll.scroll_speed_x * context.dt;
            transform.y += scroll.scroll_speed_y * context.dt;
        }
    }
}
