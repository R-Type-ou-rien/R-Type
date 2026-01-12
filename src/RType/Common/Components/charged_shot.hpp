#pragma once

struct ChargedShotComponent {
    static constexpr auto name = "ChargedShotComponent";

    bool is_charging = false;
    float charge_time = 0.0f;
    float min_charge_time = 0.5f;
    float max_charge_time = 2.0f;          // Max charge time (100% = full red bar)
    float medium_charge_threshold = 1.0f;  // Medium charged shot threshold (50% = yellow bar)
};

struct PenetratingProjectile {
    static constexpr auto name = "PenetratingProjectile";

    int max_penetrations = 999;
    int current_penetrations = 0;
};
