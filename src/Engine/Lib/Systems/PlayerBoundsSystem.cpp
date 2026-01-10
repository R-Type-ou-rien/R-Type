#include "PlayerBoundsSystem.hpp"
#include "Components/StandardComponents.hpp"

void PlayerBoundsSystem::update(Registry& registry, system_context context) {
    auto& bounds_entities = registry.getEntities<WorldBoundsComponent>();
    if (bounds_entities.empty())
        return;

    auto& bounds = registry.getConstComponent<WorldBoundsComponent>(bounds_entities[0]);

    auto& tagged_entities = registry.getEntities<TagComponent>();
    for (auto entity : tagged_entities) {
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

        if (!registry.hasComponent<transform_component_s>(entity))
            continue;

        auto& transform = registry.getComponent<transform_component_s>(entity);

        float sprite_w = 33.0f;
        float sprite_h = 17.0f;
        float scale_x = transform.scale_x;
        float scale_y = transform.scale_y;

        if (registry.hasComponent<sprite2D_component_s>(entity)) {
            auto& sprite = registry.getConstComponent<sprite2D_component_s>(entity);
            if (sprite.is_animated && !sprite.frames.empty()) {
                const auto& frame = sprite.frames[sprite.current_animation_frame];
                sprite_w = frame.width;
                sprite_h = frame.height;
            } else if (sprite.dimension.width > 0 && sprite.dimension.height > 0) {
                sprite_w = sprite.dimension.width;
                sprite_h = sprite.dimension.height;
            }
        }

        float actual_width = sprite_w * std::abs(scale_x);
        float actual_height = sprite_h * std::abs(scale_y);

        if (transform.x < bounds.min_x)
            transform.x = bounds.min_x;
        if (transform.x > bounds.max_x - actual_width)
            transform.x = bounds.max_x - actual_width;
        if (transform.y < bounds.min_y)
            transform.y = bounds.min_y;
        if (transform.y > bounds.max_y - actual_height)
            transform.y = bounds.max_y - actual_height;
    }
}
