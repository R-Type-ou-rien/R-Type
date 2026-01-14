#pragma once

struct PowerUpComponent {
    static constexpr auto name = "PowerUpComponent";
    
    enum PowerUpType {
        SPEED_UP,
        FIRE_RATE,
        SHIELD,
        WEAPON_UPGRADE
    };
    
    PowerUpType type;
    float duration = 0.0f;
    float value = 1.0f;
};

struct ActivePowerUpComponent {
    static constexpr auto name = "ActivePowerUpComponent";
    
    PowerUpComponent::PowerUpType type;
    float remaining_time;
    float original_value;
};
