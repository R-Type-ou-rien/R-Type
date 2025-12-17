/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ScrollSystem.cpp
*/

#include "ScrollSystem.hpp"
#include "registry.hpp"

void ScrollSystem::update(Registry& registry, system_context context) {
    auto& scrolls = registry.getView<Scroll>();
    const auto& entities = registry.getEntities<Scroll>();

    for (size_t i = 0; i < entities.size(); ++i) {
        Entity entity = entities[i];

        if (registry.hasComponent<transform_component_s>(entity)) {
            auto& scroll = scrolls[i];
            if (scroll.is_paused)
                continue;

            auto& transform = registry.getComponent<transform_component_s>(entity);
            transform.x += scroll.scroll_speed_x * context.dt;
            transform.y += scroll.scroll_speed_y * context.dt;
        }
    }
}
