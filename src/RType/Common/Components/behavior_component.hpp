#pragma once

struct BehaviorComponent {
    static constexpr auto name = "BehaviorComponent";
    bool shoot_at_player = false;
    bool follow_player = false;
    float follow_speed = 100.0f;
};
