#include "PlayerBoundsSystem.hpp"
#include "Components/StandardComponents.hpp"

void PlayerBoundsSystem::update(Registry& registry, system_context context) {
    const float windowWidth = static_cast<float>(context.window.getSize().x);
    const float windowHeight = static_cast<float>(context.window.getSize().y);

    float min_x = 0.0f;
    float min_y = 0.0f;

    auto& bounds_entities = registry.getEntities<WorldBoundsComponent>();
    if (!bounds_entities.empty()) {
        auto& bounds = registry.getConstComponent<WorldBoundsComponent>(bounds_entities[0]);
        min_x = bounds.min_x;
        min_y = bounds.min_y;
    }

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
