#pragma once

#include "Components/StandardComponents.hpp"
#include "registry.hpp"
#include "../Components/config.hpp"

class AnimationHelper {
   public:
    static void setupHorizontalAnimation(Registry& registry, Entity entity, const EntityConfig& config,
                                         int num_frames = 5, float animation_speed = 0.1f) {
        // if (!registry.hasComponent<sprite2D_component_s>(entity)) {
        //     return;
        // }
        if (!registry.hasComponent<AnimatedSprite2D>(entity)) {
            return;
        }

        auto& animation = registry.getComponent<AnimatedSprite2D>(entity); 

        float sprite_w = static_cast<float>(config.sprite_w.value());
        float sprite_h = static_cast<float>(config.sprite_h.value());
        float start_x = static_cast<float>(config.sprite_x.value());
        float start_y = static_cast<float>(config.sprite_y.value());
        animation.animations.at("idle").frameDuration = animation_speed;
        animation.animations.at("idle").frames.clear();
        for (int i = 0; i < num_frames; i++) {
            animation.animations.at("idle").frames.emplace_back(start_x + i * sprite_w, start_y, sprite_w, sprite_h);
        }
        
        // 7
    }

    static void setupAnimation(Registry& registry, Entity entity, float start_x, float start_y, float width,
                               float height, int num_frames, float animation_speed = 0.1f, float padding_x = 0.0f) {
        // if (!registry.hasComponent<sprite2D_component_s>(entity)) {
        //     return;
        // }

        // auto& sprite = registry.getComponent<sprite2D_component_s>(entity);
        // sprite.is_animated = true;
        // sprite.loop_animation = true;
        // sprite.animation_speed = animation_speed;
        // sprite.frames.clear();
        // sprite.dimension = {start_x, start_y, width, height};

        // for (int i = 0; i < num_frames; i++) {
        //     sprite.frames.push_back({start_x + i * (width + padding_x), start_y, width, height});
        // }

        if (!registry.hasComponent<AnimatedSprite2D>(entity)) {
            return;
        }

        auto& animation = registry.getComponent<AnimatedSprite2D>(entity);
        
        animation.animations.at("idle").frameDuration = animation_speed;
        animation.animations.at("idle").frames.clear();

        for (int i = 0; i < num_frames; i++) {
            animation.animations.at("idle").frames.emplace_back(start_x + i * (width + padding_x), start_y, width, height);
        }
    }

    static void setupStaticSprite(Registry& registry, Entity entity, const EntityConfig& config) {
        // if (!registry.hasComponent<sprite2D_component_s>(entity)) {
        //     return;
        // }

        // auto& sprite = registry.getComponent<sprite2D_component_s>(entity);
        // sprite.is_animated = false;
        // sprite.dimension = {static_cast<float>(config.sprite_x.value()), static_cast<float>(config.sprite_y.value()),
        //                     static_cast<float>(config.sprite_w.value()), static_cast<float>(config.sprite_h.value())};
        
        if (!registry.hasComponent<Sprite2D>(entity)) {
            return;
        }

        auto& sprite = registry.getComponent<Sprite2D>(entity);
        sprite.rect = static_cast<Rect2D>(static_cast<float>(config.sprite_x.value()), static_cast<float>(config.sprite_y.value()),
                            static_cast<float>(config.sprite_w.value()), static_cast<float>(config.sprite_h.value()));
    }
};
