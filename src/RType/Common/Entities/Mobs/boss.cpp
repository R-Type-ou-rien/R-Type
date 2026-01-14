/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Boss Spawner - Flexible and configurable boss entity creation
*/

#include <string>
#include <unordered_map>
#include "all_mobs.hpp"
#include "Components/StandardComponents.hpp"
#include "ResourceConfig.hpp"
#include "../../Systems/damage.hpp"
#include "../../Systems/health.hpp"
#include "../../Systems/animation_helper.hpp"
#include "../../Systems/shooter.hpp"
#include "../../Components/team_component.hpp"
#include "../../Systems/behavior.hpp"
#include "../../Systems/score.hpp"
#include "../../Components/boss_component.hpp"

struct BossDimensions {
    float width;
    float height;
    float target_x;
    float start_x;
    float start_y;

    static BossDimensions calculate(const EntityConfig& config, float world_width, float world_height) {
        BossDimensions dimensions;
        const float scale = config.scale.value_or(1.0f);

        dimensions.width = config.sprite_w.value() * scale;
        dimensions.height = config.sprite_h.value() * scale;
        dimensions.target_x = world_width - dimensions.width - BossDefaults::Position::MARGIN_RIGHT;
        dimensions.start_x = world_width + BossDefaults::Position::SPAWN_OFFSET_X;
        dimensions.start_y = (world_height / 2.0f) - (dimensions.height / 2.0f);

        return dimensions;
    }
};

BossComponent createBossComponent(const EntityConfig& config, float target_x) {
    BossComponent boss_comp;
    boss_comp.has_arrived = false;
    boss_comp.target_x = target_x;
    boss_comp.max_phases = config.max_phases.value_or(3);
    boss_comp.attack_pattern_interval = config.attack_pattern_interval.value_or(3.0f);
    boss_comp.oscillation_amplitude = config.oscillation_amplitude.value_or(100.0f);
    boss_comp.oscillation_frequency = config.oscillation_frequency.value_or(1.0f);
    boss_comp.total_weak_points = config.total_weak_points.value_or(2);
    boss_comp.death_duration = config.death_duration.value_or(3.0f);
    boss_comp.sub_entities_config = config.boss_sub_entities;
    return boss_comp;
}

ShooterComponent createBossShooter(const EntityConfig& config) {
    ShooterComponent shooter;
    shooter.type = ShooterComponent::NORMAL;
    shooter.is_shooting = true;
    shooter.fire_rate = config.fire_rate.value();
    shooter.last_shot = 0.0f;
    shooter.projectile_damage = config.projectile_damage.value_or(25);
    shooter.projectile_scale = config.projectile_scale.value_or(3.0f);

    static const std::unordered_map<std::string, ShooterComponent::ShootPattern> pattern_map = {
        {"AIM_PLAYER", ShooterComponent::AIM_PLAYER},
        {"SPREAD",     ShooterComponent::SPREAD},
        {"STRAIGHT",   ShooterComponent::STRAIGHT}
    };

    shooter.pattern = ShooterComponent::SPREAD;

    if (config.shoot_pattern.has_value()) {
        auto it = pattern_map.find(config.shoot_pattern.value());
        if (it != pattern_map.end()) {
            shooter.pattern = it->second;
        } else {
            shooter.pattern = ShooterComponent::STRAIGHT;
        }
    }

    return shooter;
}

sprite2D_component_s createBossSprite(const EntityConfig& config, handle_t<TextureAsset> handle) {
    sprite2D_component_s sprite;
    sprite.handle = handle;
    sprite.dimension = {
        static_cast<float>(config.sprite_x.value()),
        static_cast<float>(config.sprite_y.value()),
        static_cast<float>(config.sprite_w.value()),
        static_cast<float>(config.sprite_h.value())
    };
    sprite.z_index = BossDefaults::Sprite::Z_INDEX;
    return sprite;
}

BoxCollisionComponent createBossCollision() {
    BoxCollisionComponent collision;
    collision.tagCollision.push_back("FRIENDLY_PROJECTILE");
    collision.tagCollision.push_back("PLAYER");
    return collision;
}

TagComponent createBossTags() {
    TagComponent tags;
    tags.tags.push_back("AI");
    tags.tags.push_back("BOSS");
    return tags;
}

TagComponent createTailTags() {
    TagComponent tags;
    tags.tags.push_back("BOSS_TAIL");
    tags.tags.push_back("AI");
    return tags;
}

sprite2D_component_s createTailSprite(handle_t<TextureAsset> handle, const BossTailConfig& tail_config) {
    sprite2D_component_s sprite;
    sprite.handle = handle;
    sprite.dimension = {
        tail_config.sprite_x,
        tail_config.sprite_y,
        tail_config.sprite_width,
        tail_config.sprite_height
    };
    sprite.z_index = BossDefaults::Tail::Z_INDEX;
    sprite.is_animated = false;
    sprite.loop_animation = false;
    sprite.reverse_animation = false;
    sprite.animation_speed = 0.0f;
    sprite.current_animation_frame = 0;
    sprite.last_animation_update = 0.0f;
    sprite.lastUpdateTime = 0.0f;
    return sprite;
}

BossTailSegmentComponent createTailSegment(Entity boss_id, int index, int parent_id, 
                                            float segment_spacing, const BossTailConfig& tail_config) {
    BossTailSegmentComponent tail_comp;
    tail_comp.boss_entity_id = boss_id;
    tail_comp.segment_index = index;
    tail_comp.parent_segment_id = parent_id;
    tail_comp.sine_offset = index * tail_config.sine_phase_offset;
    tail_comp.base_offset_x = -segment_spacing;
    tail_comp.base_offset_y = 0.0f;
    return tail_comp;
}

void spawnTailSegments(Registry& registry, system_context& context, Entity boss_id,
                       const BossDimensions& dims, const EntityConfig& config, const BossTailConfig& tail_config) {
    const float boss_scale = config.scale.value_or(1.0f);
    const float tail_scale = boss_scale * tail_config.scale_multiplier;
    const float tail_size_x = tail_config.sprite_width * tail_scale;
    const float segment_spacing = tail_size_x * tail_config.spacing_ratio;

    handle_t<TextureAsset> tail_handle = context.texture_manager.load(
        tail_config.sprite_path,
        TextureAsset(tail_config.sprite_path)
    );

    int previous_segment_id = boss_id;

    for (int i = 0; i < tail_config.segment_count; i++) {
        Entity segment_id = registry.createEntity();

        const float segment_x = dims.start_x - (i + 1) * segment_spacing;
        const float segment_y = dims.start_y + (dims.height * tail_config.height_multiplier);

        registry.addComponent<transform_component_s>(segment_id, {segment_x, segment_y});
        registry.addComponent<Velocity2D>(segment_id, {-config.speed.value(), 0.0f});

        registry.addComponent<BossTailSegmentComponent>(segment_id,
            createTailSegment(boss_id, i, (i == 0) ? boss_id : previous_segment_id, segment_spacing, tail_config));

        registry.addComponent<HealthComponent>(segment_id, {
            tail_config.hp,
            tail_config.hp,
            BossDefaults::Tail::INVINCIBILITY_TIME,
            BossDefaults::Tail::DAMAGE_FLASH_DURATION
        });
        registry.addComponent<TeamComponent>(segment_id, {TeamComponent::ENEMY});
        registry.addComponent<DamageOnCollision>(segment_id, {tail_config.collision_damage});

        registry.addComponent<BoxCollisionComponent>(segment_id, createBossCollision());
        registry.addComponent<sprite2D_component_s>(segment_id, createTailSprite(tail_handle, tail_config));

        auto& segment_transform = registry.getComponent<transform_component_s>(segment_id);
        segment_transform.scale_x = tail_scale;
        segment_transform.scale_y = tail_scale;

        registry.addComponent<TagComponent>(segment_id, createTailTags());
        registry.addComponent<NetworkIdentity>(segment_id, {static_cast<uint32_t>(segment_id), 0});

        previous_segment_id = segment_id;
    }
}

void BossSpawner::spawn(Registry& registry, system_context context, float x, float y, const EntityConfig& config) {
    Entity id = registry.createEntity();

#if defined(CLIENT_BUILD)
    auto window_size = context.window.getSize();
    const BossDimensions dims = BossDimensions::calculate(config, static_cast<float>(window_size.x), static_cast<float>(window_size.y));
#else
    // Sur le serveur, on utilise une résolution de référence fixe
    const BossDimensions dims = BossDimensions::calculate(config, 1920.0f, 1080.0f);
#endif

    registry.addComponent<transform_component_s>(id, {dims.start_x, dims.start_y});
    registry.addComponent<Velocity2D>(id, {-config.speed.value(), 0.0f});

    registry.addComponent<HealthComponent>(id, {config.hp.value(), config.hp.value(), 0.0f, 2.0f});
    registry.addComponent<TeamComponent>(id, {TeamComponent::ENEMY});
    registry.addComponent<DamageOnCollision>(id, {config.damage.value()});
    registry.addComponent<ScoreValueComponent>(id, {config.score_value.value()});

    registry.addComponent<BossComponent>(id, createBossComponent(config, dims.target_x));
    registry.addComponent<ShooterComponent>(id, createBossShooter(config));

    handle_t<TextureAsset> handle = context.texture_manager.load(
        config.sprite_path.value(),
        TextureAsset(config.sprite_path.value())
    );

    registry.addComponent<sprite2D_component_s>(id, createBossSprite(config, handle));

    const int num_frames = config.animation_frames.value_or(1);
    const float anim_speed = config.animation_speed.value_or(0.1f);
    if (num_frames > 1) {
        AnimationHelper::setupHorizontalAnimation(registry, id, config, num_frames, anim_speed);
    }

    if (config.scale.has_value()) {
        auto& transform = registry.getComponent<transform_component_s>(id);
        transform.scale_x = config.scale.value();
        transform.scale_y = config.scale.value();
    }

    registry.addComponent<BoxCollisionComponent>(id, createBossCollision());
    registry.addComponent<TagComponent>(id, createBossTags());
    registry.addComponent<NetworkIdentity>(id, {static_cast<uint32_t>(id), 0});

    BossTailConfig tail_config;
    spawnTailSegments(registry, context, id, dims, config, tail_config);
}
