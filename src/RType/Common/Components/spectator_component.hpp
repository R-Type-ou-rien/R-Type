/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Spectator and Game State Components
*/

#pragma once

struct SpectatorComponent {
    static constexpr auto name = "SpectatorComponent";
    
    Entity watching_player = -1;
    bool is_spectating = true;
    float spectate_start_time = 0.0f;
};

struct GameStateComponent {
    static constexpr auto name = "GameStateComponent";
    
    int total_players = 0;
    int alive_players = 0;
    int dead_players = 0;
    bool all_players_dead = false;
    bool boss_defeated = false;
    bool transition_to_next_level = false;
    float transition_start_time = 0.0f;
};
