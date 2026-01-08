#pragma once

#include "Components/StandardComponents.hpp"
#include "registry.hpp"
#include "../Components/config.hpp"

class AnimationHelper {
   public:
    static void setupHorizontalAnimation(Registry& registry, Entity entity, const EntityConfig& config,
                                         int num_frames = 5, float animation_speed = 0.1f) {
        if (!registry.hasComponent<sprite2D_component_s>(entity)) {
            return;
        }

        auto& sprite = registry.getComponent<sprite2D_component_s>(entity);
        sprite.is_animated = true;
        sprite.loop_animation = true;
        sprite.animation_speed = animation_speed;
        sprite.frames.clear();

        float sprite_w = static_cast<float>(config.sprite_w.value());
        float sprite_h = static_cast<float>(config.sprite_h.value());
        float start_x = static_cast<float>(config.sprite_x.value());
        float start_y = static_cast<float>(config.sprite_y.value());

        for (int i = 0; i < num_frames; i++) {
            sprite.frames.push_back({start_x + i * sprite_w, start_y, sprite_w, sprite_h});
        }
    }

    static void setupStaticSprite(Registry& registry, Entity entity, const EntityConfig& config) {
        if (!registry.hasComponent<sprite2D_component_s>(entity)) {
            return;
        }

        auto& sprite = registry.getComponent<sprite2D_component_s>(entity);
        sprite.is_animated = false;
        sprite.dimension = {static_cast<float>(config.sprite_x.value()), static_cast<float>(config.sprite_y.value()),
                            static_cast<float>(config.sprite_w.value()), static_cast<float>(config.sprite_h.value())};
    }
};
