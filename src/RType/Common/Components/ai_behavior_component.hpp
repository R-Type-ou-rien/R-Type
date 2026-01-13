#pragma once

// AIBehaviorComponent - Component for enemies that target players
struct AIBehaviorComponent {
    static constexpr auto name = "AIBehaviorComponent";
    bool shoot_at_player = false;
    bool follow_player = false;
    float follow_speed = 100.0f;
};

// BossComponent - Component to mark the boss (useful for stopping background)
struct BossComponent {
    static constexpr auto name = "BossComponent";

    enum BossState { SPAWN, PHASE_1, PHASE_2, PHASE_3, ENRAGED, DYING, DEAD };

    bool has_arrived = false;
    float target_x = 0.0f;

    // Machine à états
    BossState current_state = SPAWN;
    float state_timer = 0.0f;

    // Système de phases
    int current_phase = 1;
    int max_phases = 3;
    float phase_transition_timer = 0.0f;

    // Patterns d'attaque
    float attack_pattern_timer = 0.0f;
    float attack_pattern_interval = 3.0f;
    int current_attack_pattern = 0;

    // Mouvement
    float oscillation_timer = 0.0f;
    float oscillation_amplitude = 100.0f;
    float oscillation_frequency = 1.0f;
    float base_y = 0.0f;

    // Points faibles
    bool has_weak_points = true;
    int weak_points_destroyed = 0;
    int total_weak_points = 2;
    bool core_vulnerable = true;

    // Phase ENRAGED
    bool is_enraged = false;

    // Dying sequence
    float death_timer = 0.0f;
    float death_duration = 3.0f;
    
    // Damage flash effect
    float damage_flash_timer = 0.0f;
    float damage_flash_duration = 0.1f;
};
