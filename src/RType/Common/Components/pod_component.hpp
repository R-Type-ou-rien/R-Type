#pragma once

enum class PodState { FLOATING, ATTACHED, DETACHED };

struct PodComponent {
    static constexpr auto name = "PodComponent";

    PodState state = PodState::FLOATING;
    Entity owner_id = -1;
    float auto_fire_rate = 0.6f;
    float last_shot_time = 0.0f;
    int projectile_damage = 15;
    float float_time = 0.0f;
    float wave_amplitude = 50.0f;
    float wave_frequency = 2.0f;
    float base_y = 0.0f;
};

struct PlayerPodComponent {
    static constexpr auto name = "PlayerPodComponent";

    bool has_pod = false;
    Entity pod_entity = -1;
    bool pod_attached = true;
    bool detach_requested = false;
    int last_known_hp = -1;

    float pod_laser_fire_rate = 1.5f;
    float pod_laser_cooldown = 1.0f;
};

struct PodSpawnComponent {
    static constexpr auto name = "PodSpawnComponent";

    float spawn_timer = 0.0f;
    float spawn_interval = 15.0f;
    float min_spawn_interval = 10.0f;
    float max_spawn_interval = 20.0f;
    bool can_spawn = true;
};
