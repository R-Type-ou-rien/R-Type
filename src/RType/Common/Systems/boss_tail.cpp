#include "boss_tail.hpp"
#include "Components/StandardComponents.hpp"
#include "../Components/boss_component.hpp"
#include <cmath>
#include <iostream>

void BossTailSystem::update(Registry& registry, system_context context) {
    auto& tail_segments = registry.getEntities<BossTailSegmentComponent>();
    
    // Animation parameters
    static float global_time = 0.0f;
    global_time += context.dt;
    
    const float wave_frequency = 2.0f;    // Oscillations per second
    const float wave_amplitude = 50.0f;   // Vertical wave amplitude in pixels
    const float follow_speed = 8.0f;      // How quickly segments follow their parent
    
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
        if (!registry.hasComponent<transform_component_s>(segment_entity))
            continue;
            
        auto& segment_transform = registry.getComponent<transform_component_s>(segment_entity);
        
        // Determine parent position
        transform_component_s parent_transform;
        bool parent_exists = false;
        
        if (tail_comp.segment_index == 0) {
            // First segment follows the boss directly
            if (registry.hasComponent<transform_component_s>(tail_comp.boss_entity_id)) {
                parent_transform = registry.getConstComponent<transform_component_s>(tail_comp.boss_entity_id);
                parent_exists = true;
                
                // Get boss dimensions for proper attachment
                if (registry.hasComponent<sprite2D_component_s>(tail_comp.boss_entity_id)) {
                    auto& boss_sprite = registry.getConstComponent<sprite2D_component_s>(tail_comp.boss_entity_id);
                    // Attach to the back/left of the boss
                    parent_transform.x -= boss_sprite.dimension.width * parent_transform.scale_x * 0.3f;
                }
            }
        } else {
            // Other segments follow the previous segment
            if (registry.hasComponent<transform_component_s>(tail_comp.parent_segment_id)) {
                parent_transform = registry.getConstComponent<transform_component_s>(tail_comp.parent_segment_id);
                parent_exists = true;
            }
        }
        
        if (!parent_exists) {
            std::cerr << "⚠️ Tail segment " << segment_entity << " has no parent!" << std::endl;
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
