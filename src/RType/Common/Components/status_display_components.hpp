/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Status Display Components for R-Type style UI
*/

#pragma once

#include "ECS/Utils/slot_map/slot_map.hpp"

struct StatusDisplayComponent {
    static constexpr auto name = "StatusDisplayComponent";

    Entity player_entity = -1;
    bool is_initialized = false;
};

// Layout: écran 1920x1080, barre de status en bas
// Division en 3 sections égales avec espacement
// Section 1 (Lives): 20 à 620 (600px)
// Section 2 (Charge): 660 à 1260 (600px)
// Section 3 (Score): 1300 à 1900 (600px)
// Espacement entre sections: 40px, marges: 20px

constexpr float STATUS_BAR_Y = 1020.0f;
constexpr float STATUS_BAR_HEIGHT = 50.0f;
constexpr float SECTION_WIDTH = 600.0f;
constexpr float SECTION_SPACING = 40.0f;
constexpr float MARGIN = 20.0f;

struct ChargeBarComponent {
    static constexpr auto name = "ChargeBarComponent";

    float current_charge = 0.0f;
    float max_charge = 1.0f;
    // Section centrale: 660 à 1260, centré dans cette zone
    float bar_width = 500.0f;
    float bar_height = 30.0f;
    float x = 710.0f;  // 660 + 50 pour centrer la barre de 500px
    float y = STATUS_BAR_Y;
};

struct LivesDisplayComponent {
    static constexpr auto name = "LivesDisplayComponent";

    int current_lives = 3;
    int max_lives = 3;
    // Section gauche: 20 à 620
    float x = MARGIN + 50.0f;  // 70
    float y = STATUS_BAR_Y;
    float icon_size = 40.0f;
    float icon_spacing = 50.0f;
};

struct ScoreDisplayComponent {
    static constexpr auto name = "ScoreDisplayComponent";

    int current_score = 0;
    int digit_count = 7;
    // Section droite: 1300 à 1900, centré
    float x = 1450.0f;
    float y = STATUS_BAR_Y;
};
