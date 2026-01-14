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

    namespace Patterns {
        constexpr float LINEAR_SPEED = -400.0f;
        constexpr int LINEAR_DAMAGE = 25;
        
        constexpr float MISSILE_SPEED = -250.0f;
        constexpr float MISSILE_OFFSET_Y = 100.0f;
        constexpr int MISSILE_DAMAGE = 30;

        constexpr float WALL_ANGLE_STEP = 15.0f;
        constexpr float WALL_SPEED = -350.0f;
        constexpr int WALL_DAMAGE = 35;
        constexpr int WALL_COUNT_SIDE = 2;

        constexpr float BOUNCE_SPEED_X = -300.0f;
        constexpr float BOUNCE_OFFSET_Y = 200.0f;
        constexpr int BOUNCE_DAMAGE = 35;

        constexpr float SPIRAL_SPEED = -300.0f;
        constexpr int SPIRAL_DAMAGE = 40;
        constexpr int SPIRAL_COUNT = 8;
        constexpr float SPIRAL_ROTATION_SPEED = 180.0f;

        constexpr float DELAYED_SPEED = -200.0f;
        constexpr int DELAYED_DAMAGE = 45;
    }

    namespace Projectile {
        constexpr float OFFSET_X = -30.0f;
        constexpr float SCALE = 2.0f;
        constexpr float SPRITE_X = 241.0f;
        constexpr float SPRITE_Y = 120.0f;
        constexpr float SPRITE_W = 10.0f;
        constexpr float SPRITE_H = 10.0f;
        constexpr int Z_INDEX = 5;
        inline const char* SPRITE_PATH = "src/RType/Common/content/sprites/r-typesheet1.gif";
    }

    namespace SubEntities {
        constexpr float PROJECTILE_SPEED = -400.0f;
        constexpr int PROJECTILE_DAMAGE = 20;
        
        namespace Tentacle {
            constexpr int HP = 50;
            constexpr int COLLISION_DAMAGE = 15;
        }
        namespace Cannon {
            constexpr int HP = 80;
            constexpr int COLLISION_DAMAGE = 20;
        }
    }
    
    namespace Phases {
        constexpr float FIRE_RATE_PHASE_2 = 0.2f;
        constexpr float FIRE_RATE_PHASE_3 = 0.15f;
        constexpr float FIRE_RATE_ENRAGED = 0.1f;
        constexpr float ENRAGED_SPEED_FACTOR = 2.0f;
        constexpr float ENRAGED_AMPLITUDE = 150.0f;
        constexpr float ENRAGED_ATTACK_INTERVAL = 0.8f;
        constexpr float DYING_FALL_SPEED = 500.0f;
        constexpr float PHASE3_VULNERABILITY_START = 1.0f;
        constexpr float PHASE3_VULNERABILITY_END = 2.0f;
    }
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

struct BossPatternConfig {
    float linear_speed = BossDefaults::Patterns::LINEAR_SPEED;
    int linear_damage = BossDefaults::Patterns::LINEAR_DAMAGE;
    
    float missile_speed = BossDefaults::Patterns::MISSILE_SPEED;
    float missile_offset_y = BossDefaults::Patterns::MISSILE_OFFSET_Y;
    int missile_damage = BossDefaults::Patterns::MISSILE_DAMAGE;

    float wall_angle_step = BossDefaults::Patterns::WALL_ANGLE_STEP;
    float wall_speed = BossDefaults::Patterns::WALL_SPEED;
    int wall_damage = BossDefaults::Patterns::WALL_DAMAGE;
    int wall_count_side = BossDefaults::Patterns::WALL_COUNT_SIDE;

    float bounce_speed_x = BossDefaults::Patterns::BOUNCE_SPEED_X;
    float bounce_offset_y = BossDefaults::Patterns::BOUNCE_OFFSET_Y;
    int bounce_damage = BossDefaults::Patterns::BOUNCE_DAMAGE;

    float spiral_speed = BossDefaults::Patterns::SPIRAL_SPEED;
    int spiral_damage = BossDefaults::Patterns::SPIRAL_DAMAGE;
    int spiral_count = BossDefaults::Patterns::SPIRAL_COUNT;
    float spiral_rotation_speed = BossDefaults::Patterns::SPIRAL_ROTATION_SPEED;

    float delayed_speed = BossDefaults::Patterns::DELAYED_SPEED;
    int delayed_damage = BossDefaults::Patterns::DELAYED_DAMAGE;
};

struct BossPhaseConfig {
    float fire_rate_phase_2 = BossDefaults::Phases::FIRE_RATE_PHASE_2;
    float fire_rate_phase_3 = BossDefaults::Phases::FIRE_RATE_PHASE_3;
    float fire_rate_enraged = BossDefaults::Phases::FIRE_RATE_ENRAGED;
    float enraged_speed_factor = BossDefaults::Phases::ENRAGED_SPEED_FACTOR;
    float enraged_amplitude = BossDefaults::Phases::ENRAGED_AMPLITUDE;
    float enraged_attack_interval = BossDefaults::Phases::ENRAGED_ATTACK_INTERVAL;
    float dying_fall_speed = BossDefaults::Phases::DYING_FALL_SPEED;
    float phase3_vulnerability_start = BossDefaults::Phases::PHASE3_VULNERABILITY_START;
    float phase3_vulnerability_end = BossDefaults::Phases::PHASE3_VULNERABILITY_END;
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
    
    BossPatternConfig patterns;
    BossPhaseConfig phases;

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
