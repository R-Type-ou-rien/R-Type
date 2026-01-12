#include <string>
#include "all_mobs.hpp"
#include "Components/StandardComponents.hpp"
#include "ResourceConfig.hpp"
#include "../../Systems/damage.hpp"
#include "../../Systems/health.hpp"
#include "../../Systems/animation_helper.hpp"
#include "../../Systems/shooter.hpp"
#include "../../Components/team_component.hpp"
#include "../../Systems/ai_behavior.hpp"
#include "../../Systems/score.hpp"
#include "../../Components/boss_component.hpp"

void BossSpawner::spawn(Registry& registry, system_context context, float x, float y, const EntityConfig& config) {
    Entity id = registry.createEntity();

    float windowWidth = 1920.0f;
    float windowHeight = 1080.0f;

    float boss_width = config.sprite_w.value() * config.scale.value_or(1.0f);
    float boss_height = config.sprite_h.value() * config.scale.value_or(1.0f);
    float boss_target_x = windowWidth - boss_width - 100.0f;
    float start_x = windowWidth + 50.0f;
    float start_y = (windowHeight / 2.0f) - (boss_height / 2.0f);

    registry.addComponent<transform_component_s>(id, {start_x, start_y});
    registry.addComponent<Velocity2D>(id, {-config.speed.value(), 0.0f});

    registry.addComponent<HealthComponent>(id, {config.hp.value(), config.hp.value(), 0.0f, 2.0f});
    registry.addComponent<TeamComponent>(id, {TeamComponent::ENEMY});
    registry.addComponent<DamageOnCollision>(id, {config.damage.value()});
    registry.addComponent<ScoreValueComponent>(id, {config.score_value.value()});

    BossComponent boss_comp;
    boss_comp.has_arrived = false;
    boss_comp.target_x = boss_target_x;
    registry.addComponent<BossComponent>(id, boss_comp);

    ShooterComponent boss_shooter;
    boss_shooter.type = ShooterComponent::NORMAL;
    boss_shooter.is_shooting = true;
    boss_shooter.fire_rate = config.fire_rate.value();
    boss_shooter.last_shot = 0.0f;
    boss_shooter.projectile_damage = config.projectile_damage.value_or(25);

    if (config.shoot_pattern.has_value()) {
        std::string pattern = config.shoot_pattern.value();
        if (pattern == "AIM_PLAYER") {
            boss_shooter.pattern = ShooterComponent::AIM_PLAYER;
        } else if (pattern == "SPREAD") {
            boss_shooter.pattern = ShooterComponent::SPREAD;
        } else {
            boss_shooter.pattern = ShooterComponent::STRAIGHT;
        }
    } else {
        boss_shooter.pattern = ShooterComponent::SPREAD;  // Boss tire en éventail par défaut
    }

    registry.addComponent<ShooterComponent>(id, boss_shooter);

    handle_t<TextureAsset> handle =
        context.texture_manager.load(config.sprite_path.value(), TextureAsset(config.sprite_path.value()));

    sprite2D_component_s sprite_info;
    sprite_info.handle = handle;
    sprite_info.dimension = {static_cast<float>(config.sprite_x.value()), static_cast<float>(config.sprite_y.value()),
                             static_cast<float>(config.sprite_w.value()), static_cast<float>(config.sprite_h.value())};

    sprite_info.z_index = 2;
    registry.addComponent<sprite2D_component_s>(id, sprite_info);

    int num_frames = config.animation_frames.value_or(1);
    float anim_speed = config.animation_speed.value_or(0.1f);
    if (num_frames > 1) {
        AnimationHelper::setupHorizontalAnimation(registry, id, config, num_frames, anim_speed);
    }

    if (config.scale.has_value()) {
        auto& transform = registry.getComponent<transform_component_s>(id);
        transform.scale_x = config.scale.value();
        transform.scale_y = config.scale.value();
    }

    BoxCollisionComponent collision;
    collision.tagCollision.push_back("FRIENDLY_PROJECTILE");
    collision.tagCollision.push_back("PLAYER");
    registry.addComponent<BoxCollisionComponent>(id, collision);

    TagComponent tags;
    tags.tags.push_back("AI");
    tags.tags.push_back("BOSS");
    registry.addComponent<TagComponent>(id, tags);

    // Add NetworkIdentity for network replication
    registry.addComponent<NetworkIdentity>(id, {static_cast<uint32_t>(id), 0});
    
    // Create tail segments (10 segments for serpentine effect)
    const int num_tail_segments = 10;
    const float segment_spacing = 60.0f;  // Distance between segments
    const float tail_width = 40.0f;
    const float tail_height = 40.0f;
    
    int previous_segment_id = id;  // First segment follows the boss
    
    for (int i = 0; i < num_tail_segments; i++) {
        Entity segment_id = registry.createEntity();
        
        // Position initiale derrière le boss
        float segment_x = start_x - (i + 1) * segment_spacing;
        float segment_y = start_y + (boss_height / 2.0f) - (tail_height / 2.0f);
        
        registry.addComponent<transform_component_s>(segment_id, {segment_x, segment_y});
        registry.addComponent<Velocity2D>(segment_id, {-config.speed.value(), 0.0f});
        
        // Tail segment component
        BossTailSegmentComponent tail_comp;
        tail_comp.boss_entity_id = id;
        tail_comp.segment_index = i;
        tail_comp.parent_segment_id = (i == 0) ? id : previous_segment_id;
        tail_comp.sine_offset = i * 0.5f;  // Phase offset for wave propagation
        tail_comp.base_offset_x = -segment_spacing;
        tail_comp.base_offset_y = 0.0f;
        registry.addComponent<BossTailSegmentComponent>(segment_id, tail_comp);
        
        // Health and team
        registry.addComponent<HealthComponent>(segment_id, {50, 50, 0.0f, 0.5f});
        registry.addComponent<TeamComponent>(segment_id, {TeamComponent::ENEMY});
        registry.addComponent<DamageOnCollision>(segment_id, {30});  // Tail damages player
        
        // Collision
        BoxCollisionComponent tail_collision;
        tail_collision.tagCollision.push_back("FRIENDLY_PROJECTILE");
        tail_collision.tagCollision.push_back("PLAYER");
        registry.addComponent<BoxCollisionComponent>(segment_id, tail_collision);
        
        // Sprite (use simple sprite for now, can be customized)
        handle_t<TextureAsset> tail_handle =
            context.texture_manager.load(config.sprite_path.value(), TextureAsset(config.sprite_path.value()));
        
        sprite2D_component_s tail_sprite;
        tail_sprite.handle = tail_handle;
        tail_sprite.dimension = {static_cast<float>(config.sprite_x.value()), 
                                static_cast<float>(config.sprite_y.value()),
                                tail_width, tail_height};
        tail_sprite.z_index = 1;  // Behind boss
        tail_sprite.is_animated = false;  // CRITICAL: Prevent animation system from accessing empty frames vector
        tail_sprite.loop_animation = false;
        tail_sprite.reverse_animation = false;
        tail_sprite.animation_speed = 0.0f;
        tail_sprite.current_animation_frame = 0;
        tail_sprite.last_animation_update = 0.0f;
        tail_sprite.lastUpdateTime = 0.0f;
        registry.addComponent<sprite2D_component_s>(segment_id, tail_sprite);
        
        if (config.scale.has_value()) {
            auto& segment_transform = registry.getComponent<transform_component_s>(segment_id);
            segment_transform.scale_x = config.scale.value() * 0.8f;  // Slightly smaller
            segment_transform.scale_y = config.scale.value() * 0.8f;
        }
        
        // Tags
        TagComponent segment_tags;
        segment_tags.tags.push_back("BOSS_TAIL");
        segment_tags.tags.push_back("AI");
        registry.addComponent<TagComponent>(segment_id, segment_tags);
        
        // Network identity
        registry.addComponent<NetworkIdentity>(segment_id, {static_cast<uint32_t>(segment_id), 0});
        
        previous_segment_id = segment_id;
    }
}
