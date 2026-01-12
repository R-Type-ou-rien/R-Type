/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Game Over Notification Component - Signal from network that game is over
*/

#pragma once

#include "ECS.hpp"

/**
 * Composant créé côté client quand on reçoit S_GAME_OVER du serveur
 * Le GameManager détecte ce composant et affiche le leaderboard
 */
struct GameOverNotification {
    static constexpr auto name = "GameOverNotification";
    bool victory;  // true = victoire, false = défaite
};
