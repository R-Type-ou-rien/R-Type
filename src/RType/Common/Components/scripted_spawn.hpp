/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Scripted Spawn Component - For authentic R-Type level spawns
*/

#pragma once

#include <string>
#include <vector>

struct SpawnEvent {
    float trigger_time;
    std::string enemy_type;
    float x_position;
    float y_position;
    int count;
    float spacing;
    std::string formation;
    float custom_speed;
    int custom_hp;
    bool executed;
};

struct ScriptedSpawnComponent {
    static constexpr auto name = "ScriptedSpawnComponent";

    std::vector<SpawnEvent> spawn_events;
    std::string script_path;
    int next_event_index = 0;
    bool all_events_completed = false;
    float level_time = 0.0f;
};
