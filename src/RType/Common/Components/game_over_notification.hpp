/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Game Over Notification Component - Signal from network that game is over
*/

#pragma once

struct GameOverNotification {
    static constexpr auto name = "GameOverNotification";
    bool victory;
};
