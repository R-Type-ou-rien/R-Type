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

struct ChargeBarComponent {
    static constexpr auto name = "ChargeBarComponent";

    float current_charge = 0.0f;
    float max_charge = 1.0f;
    float bar_width = 200.0f;
    float bar_height = 20.0f;
    float x = 860.0f;
    float y = 1030.0f;
};

struct LivesDisplayComponent {
    static constexpr auto name = "LivesDisplayComponent";

    int current_lives = 3;
    int max_lives = 3;
    float x = 50.0f;
    float y = 1030.0f;
    float icon_size = 32.0f;
    float icon_spacing = 40.0f;
};

struct ScoreDisplayComponent {
    static constexpr auto name = "ScoreDisplayComponent";

    int current_score = 0;
    int digit_count = 7;
    float x = 1650.0f;
    float y = 1030.0f;
};
