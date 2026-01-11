/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Scripted Spawn Component - For authentic R-Type level spawns
*/

#pragma once

#include <string>
#include <vector>

// Individual spawn event in the level
struct SpawnEvent {
    float trigger_time;      // Time in seconds when this spawn occurs
    std::string enemy_type;  // SCOUT, FIGHTER, TANK, SHOOTER, KAMIKAZE, etc.
    float x_position;        // X spawn position
    float y_position;        // Y spawn position
    int count;               // Number of enemies to spawn
    float spacing;           // Spacing between enemies in formation
    std::string formation;   // SINGLE, LINE_HORIZONTAL, LINE_VERTICAL, V_FORMATION, SNAKE
    float custom_speed;      // Optional custom speed (0 = use default)
    int custom_hp;           // Optional custom HP (0 = use default)
    bool executed;           // Has this event been executed?
};

// Component to track scripted spawns for a level
struct ScriptedSpawnComponent {
    static constexpr auto name = "ScriptedSpawnComponent";

    std::vector<SpawnEvent> spawn_events;
    int next_event_index = 0;
    bool all_events_completed = false;
    float level_time = 0.0f;
};

// Powerup spawn event
struct PowerupEvent {
    float trigger_time;
    std::string powerup_type;  // SPEED, MISSILE, FORCE, LASER_BLUE, LASER_RED
    float x_position;
    float y_position;
    bool executed;
};

struct ScriptedPowerupComponent {
    static constexpr auto name = "ScriptedPowerupComponent";

    std::vector<PowerupEvent> powerup_events;
    int next_event_index = 0;
};
