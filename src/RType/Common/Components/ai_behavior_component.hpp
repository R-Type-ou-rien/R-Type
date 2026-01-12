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
    bool has_arrived = false;
    float target_x = 0.0f;
};
