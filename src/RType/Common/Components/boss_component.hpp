/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Boss Component - Flexible and configurable boss entity components
*/

#pragma once

#include <vector>
#include <string>
#include <optional>

namespace BossDefaults {
    namespace Position {
        constexpr float MARGIN_RIGHT = 100.0f;
        constexpr float SPAWN_OFFSET_X = 50.0f;
    }

    namespace Tail {
        constexpr int SEGMENT_COUNT = 18;
        constexpr float SPRITE_WIDTH = 15.0f;
        constexpr float SPRITE_HEIGHT = 15.0f;
        constexpr float SCALE_MULTIPLIER = 2.0f;
        constexpr float SPACING_RATIO = 0.4f;
        constexpr float SINE_PHASE_OFFSET = 0.5f;
        constexpr float HEIGHT_MULTIPLIER = 2.0f;
        constexpr int HP = 50;
        constexpr float INVINCIBILITY_TIME = 0.0f;
        constexpr float DAMAGE_FLASH_DURATION = 0.5f;
        constexpr int COLLISION_DAMAGE = 30;
        constexpr float SPRITE_X = 596.0f;
        constexpr float SPRITE_Y = 2061.0f;
        constexpr int Z_INDEX = 1;
    }

    namespace Sprite {
        constexpr int Z_INDEX = 2;
    }

    inline const char* TAIL_SPRITE_PATH = "src/RType/Common/content/sprites/r-typesheet30.gif";
}

struct BossTailConfig {
    int segment_count = BossDefaults::Tail::SEGMENT_COUNT;
    float sprite_width = BossDefaults::Tail::SPRITE_WIDTH;
    float sprite_height = BossDefaults::Tail::SPRITE_HEIGHT;
    float scale_multiplier = BossDefaults::Tail::SCALE_MULTIPLIER;
    float spacing_ratio = BossDefaults::Tail::SPACING_RATIO;
    float sine_phase_offset = BossDefaults::Tail::SINE_PHASE_OFFSET;
    float height_multiplier = BossDefaults::Tail::HEIGHT_MULTIPLIER;
    int hp = BossDefaults::Tail::HP;
    int collision_damage = BossDefaults::Tail::COLLISION_DAMAGE;
    float sprite_x = BossDefaults::Tail::SPRITE_X;
    float sprite_y = BossDefaults::Tail::SPRITE_Y;
    int z_index = BossDefaults::Tail::Z_INDEX;
    std::string sprite_path = BossDefaults::TAIL_SPRITE_PATH;
};

struct BossPositionConfig {
    float margin_right = BossDefaults::Position::MARGIN_RIGHT;
    float spawn_offset_x = BossDefaults::Position::SPAWN_OFFSET_X;
    int z_index = BossDefaults::Sprite::Z_INDEX;
};

struct BossSubEntityConfig {
    std::string type;
    float offset_x;
    float offset_y;
    float fire_rate;
};

struct BossComponent {
    static constexpr auto name = "BossComponent";

    enum BossState {
        SPAWN,
        PHASE_1,
        PHASE_2,
        PHASE_3,
        ENRAGED,
        DYING,
        DEAD
    };

    int max_phases = 3;
    float attack_pattern_interval = 3.0f;
    float oscillation_amplitude = 100.0f;
    float oscillation_frequency = 1.0f;
    int total_weak_points = 2;
    float death_duration = 3.0f;
    float damage_flash_duration = 0.1f;
    std::vector<BossSubEntityConfig> sub_entities_config;

    bool has_arrived = false;
    float target_x = 0.0f;
    BossState current_state = SPAWN;
    float state_timer = 0.0f;
    int current_phase = 1;
    float phase_transition_timer = 0.0f;
    float attack_pattern_timer = 0.0f;
    int current_attack_pattern = 0;
    float oscillation_timer = 0.0f;
    float base_y = 0.0f;
    bool has_weak_points = true;
    int weak_points_destroyed = 0;
    bool core_vulnerable = true;
    bool is_enraged = false;
    float death_timer = 0.0f;
    float damage_flash_timer = 0.0f;
};

struct BossWeakPointComponent {
    static constexpr auto name = "BossWeakPointComponent";
    
    int boss_entity_id;
    int weak_point_index;
    bool is_destroyed = false;
    float offset_x = 0.0f;
    float offset_y = 0.0f;
};

struct BossSubEntityComponent {
    static constexpr auto name = "BossSubEntityComponent";
    
    enum SubEntityType {
        TENTACLE,
        CANNON,
        SHIELD
    };
    
    int boss_entity_id;
    SubEntityType type;
    int sub_entity_index;
    bool is_active = true;
    bool is_destroyed = false;
    float offset_x = 0.0f;
    float offset_y = 0.0f;
    float fire_timer = 0.0f;
    float fire_rate = 2.0f;
};

struct BossTailSegmentComponent {
    static constexpr auto name = "BossTailSegmentComponent";
    
    int boss_entity_id;
    int segment_index;
    int parent_segment_id;    
    float sine_offset = 0.0f;
    float base_offset_x = 0.0f;
    float base_offset_y = 0.0f;
};
