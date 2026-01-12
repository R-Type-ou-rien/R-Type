#pragma once

#include "registry.hpp"

enum class PodState { FLOATING, ATTACHED, DETACHED };

struct PodComponent {
    static constexpr auto name = "PodComponent";  // constexpr c'est pour qu'il doit connu au moment de la compilation

    PodState state = PodState::FLOATING;
    Entity owner_id = -1;
    float auto_fire_rate = 0.6f;
    float last_shot_time = 0.0f;
    int projectile_damage = 15;

    // Floating movement parameters
    float float_time = 0.0f;
    float wave_amplitude = 50.0f;  // Amplitude of vertical wave
    float wave_frequency = 2.0f;   // Speed of wave oscillation
    float base_y = 0.0f;           // Original Y position for wave calculation
};

struct PlayerPodComponent {
    static constexpr auto name = "PlayerPodComponent";

    bool has_pod = false;
    Entity pod_entity = -1;
    bool pod_attached = true;
    bool detach_requested = false;
    int last_known_hp = -1;

    // Pod laser fire rate (when attached) - fast like R-Type
    float pod_laser_fire_rate = 0.08f;  // Time between pod laser shots (fast)
    float pod_laser_cooldown = 0.0f;    // Current cooldown timer
};

struct PodSpawnComponent {
    static constexpr auto name = "PodSpawnComponent";

    float spawn_timer = 0.0f;
    float spawn_interval = 15.0f;
    float min_spawn_interval = 10.0f;
    float max_spawn_interval = 20.0f;
    bool can_spawn = true;
};
