/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Spectator and Game State Components
*/

#pragma once

#include "ECS.hpp"

// Composant pour marquer un joueur comme spectateur
struct SpectatorComponent {
    static constexpr auto name = "SpectatorComponent";
    
    Entity watching_player = -1;  // Le joueur qu'on observe
    bool is_spectating = true;
    float spectate_start_time = 0.0f;
};

// Composant pour tracker l'état de la partie multijoueur
struct GameStateComponent {
    static constexpr auto name = "GameStateComponent";
    
    int total_players = 0;        // Nombre total de joueurs au début
    int alive_players = 0;        // Nombre de joueurs encore vivants
    int dead_players = 0;         // Nombre de joueurs morts
    bool all_players_dead = false;
    bool boss_defeated = false;
    bool transition_to_next_level = false;
    float transition_start_time = 0.0f;
};

// Composant pour l'animation de transition entre niveaux
struct LevelTransitionComponent {
    static constexpr auto name = "LevelTransitionComponent";
    
    enum class TransitionState {
        IDLE,
        FADING_OUT,
        SHOW_LEVEL_COMPLETE,
        FADING_IN,
        COMPLETE
    };
    
    TransitionState state = TransitionState::IDLE;
    float fade_alpha = 0.0f;
    float transition_time = 0.0f;
    float fade_duration = 1.0f;
    float display_duration = 2.0f;
    std::string next_level_name;
};
