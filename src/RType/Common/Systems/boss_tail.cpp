#include "boss_tail.hpp"
#include "Components/StandardComponents.hpp"
#include "../Components/boss_component.hpp"
#include <cmath>

void BossTailSystem::update(Registry& registry, system_context context) {
    auto& tail_segments = registry.getEntities<BossTailSegmentComponent>();

    // Animation parameters
    static float global_time = 0.0f;
    global_time += context.dt;

    const float wave_frequency = 2.0f;   // Oscillations per second
    const float wave_amplitude = 25.0f;  // Smaller amplitude keeps the chain tighter
    const float follow_speed = 14.0f;    // Follow faster to avoid "stretching" gaps

    for (auto segment_entity : tail_segments) {
        if (!registry.hasComponent<BossTailSegmentComponent>(segment_entity))
            continue;

        auto& tail_comp = registry.getComponent<BossTailSegmentComponent>(segment_entity);

        // Check if boss still exists
        if (!registry.hasComponent<BossComponent>(tail_comp.boss_entity_id)) {
            // Boss is dead, destroy tail segment
            registry.addComponent<PendingDestruction>(segment_entity, {});
            continue;
        }

        // Get segment transform
        if (!registry.hasComponent<TransformComponent>(segment_entity))
            continue;

        auto& segment_transform = registry.getComponent<TransformComponent>(segment_entity);

        // Determine parent position
        TransformComponent parent_transform;
        bool parent_exists = false;

        if (tail_comp.segment_index == 0) {
            // First segment follows the boss directly
            if (registry.hasComponent<TransformComponent>(tail_comp.boss_entity_id)) {
                parent_transform = registry.getConstComponent<TransformComponent>(tail_comp.boss_entity_id);
                parent_exists = true;

                // Attach point: middle-left of the boss sprite
                // if (registry.hasComponent<Sprite2DComponent>(tail_comp.boss_entity_id)) {
                //     auto& boss_sprite = registry.getConstComponent<Sprite2DComponent>(tail_comp.boss_entity_id);

                //     const float boss_w = boss_sprite.dimension.width * parent_transform.scale_x;
                //     const float boss_h = boss_sprite.dimension.height * parent_transform.scale_y;

                //     // User tuning: move attachment more to the right and lower
                //     float anchor_x = parent_transform.x + (boss_w * 0.40f);
                //     float anchor_y = parent_transform.y + (boss_h * 0.85f);

                //     // Center the segment sprite on the anchor point (if available)
                //     if (registry.hasComponent<Sprite2DComponent>(segment_entity)) {
                //         auto& seg_sprite = registry.getConstComponent<Sprite2DComponent>(segment_entity);
                //         const float seg_w = seg_sprite.dimension.width * segment_transform.scale_x;
                //         const float seg_h = seg_sprite.dimension.height * segment_transform.scale_y;
                //         anchor_x -= seg_w * 0.50f;
                //         anchor_y -= seg_h * 0.50f;
                //     }

                //     parent_transform.x = anchor_x;
                //     parent_transform.y = anchor_y;
                // }
                if (registry.hasComponent<AnimatedSprite2D>(tail_comp.boss_entity_id)) {
                    auto& boss_sprite = registry.getConstComponent<AnimatedSprite2D>(tail_comp.boss_entity_id);

                    const float boss_w = boss_sprite.animations.at(boss_sprite.currentAnimation).frames.at(boss_sprite.currentFrameIndex).width * parent_transform.scale_x;
                    const float boss_h = boss_sprite.animations.at(boss_sprite.currentAnimation).frames.at(boss_sprite.currentFrameIndex).height * parent_transform.scale_y;

                    // User tuning: move attachment more to the right and lower
                    float anchor_x = parent_transform.x + (boss_w * 0.40f);
                    float anchor_y = parent_transform.y + (boss_h * 0.85f);

                    // Center the segment sprite on the anchor point (if available)
                    if (registry.hasComponent<AnimatedSprite2D>(segment_entity)) {
                        auto& seg_sprite = registry.getConstComponent<AnimatedSprite2D>(segment_entity);
                        const float seg_w = seg_sprite.animations.at(seg_sprite.currentAnimation).frames.at(seg_sprite.currentFrameIndex).width * segment_transform.scale_x;
                        const float seg_h = seg_sprite.animations.at(seg_sprite.currentAnimation).frames.at(seg_sprite.currentFrameIndex).height * segment_transform.scale_y;
                        anchor_x -= seg_w * 0.50f;
                        anchor_y -= seg_h * 0.50f;
                    }

                    parent_transform.x = anchor_x;
                    parent_transform.y = anchor_y;
                }
            }
        } else {
            // Other segments follow the previous segment
            if (registry.hasComponent<TransformComponent>(tail_comp.parent_segment_id)) {
                parent_transform = registry.getConstComponent<TransformComponent>(tail_comp.parent_segment_id);
                parent_exists = true;
            }
        }

        if (!parent_exists) {
            continue;
        }

        // Calculate target position with sine wave
        float sine_wave = std::sin((global_time * wave_frequency) + tail_comp.sine_offset) * wave_amplitude;

        float target_x = parent_transform.x + tail_comp.base_offset_x;
        float target_y = parent_transform.y + tail_comp.base_offset_y + sine_wave;

        // Smooth interpolation towards target (serpentine following)
        float dx = target_x - segment_transform.x;
        float dy = target_y - segment_transform.y;

        segment_transform.x += dx * follow_speed * context.dt;
        segment_transform.y += dy * follow_speed * context.dt;
    }
}
