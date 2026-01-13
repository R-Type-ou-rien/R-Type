#pragma once

#include <string>

struct PowerUpComponent {
    static constexpr auto name = "PowerUpComponent";
    
    enum PowerUpType {
        SPEED_UP,
        FIRE_RATE,
        SHIELD,
        WEAPON_UPGRADE
    };
    
    PowerUpType type;
    float duration = 0.0f;  // 0 = permanent, > 0 = temporaire
    float value = 1.0f;     // Multiplicateur ou valeur du power-up
};

struct ActivePowerUpComponent {
    static constexpr auto name = "ActivePowerUpComponent";
    
    PowerUpComponent::PowerUpType type;
    float remaining_time;
    float original_value;  // Valeur originale avant le power-up (pour restaurer)
};
