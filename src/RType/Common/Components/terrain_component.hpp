/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** TerrainComponent - Component for wall/terrain elements
*/

#pragma once

struct WallComponent {
    static constexpr auto name = "WallComponent";
    bool is_destructible = false;
    bool blocks_projectiles = true;
    bool blocks_player = true;
    bool blocks_enemies = true;
    float width = 0.0f;
    float height = 0.0f;
};
