#pragma once

struct ChargedShotComponent {
    static constexpr auto name = "ChargedShotComponent";

    bool is_charging = false;
    float charge_time = 0.0f;
    float min_charge_time = 0.5f;
    float max_charge_time = 2.0f;
    float medium_charge = 1.0f;
};

struct PenetratingProjectile {
    static constexpr auto name = "PenetratingProjectile";

    int max_penetrations = 999;
    int current_penetrations = 0;
};
