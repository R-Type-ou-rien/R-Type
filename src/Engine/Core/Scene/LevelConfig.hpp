#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <any>

struct SceneEntityConfig {
    std::string type;
    std::unordered_map<std::string, std::any> properties;
};

struct LevelConfig {
    std::string name;
    std::string background_texture;
    std::string music_track;

    // Additional configuration paths for game modularity
    std::string enemies_config;
    std::string boss_config;
    std::string boss_section = "DEFAULT";
    std::string game_config;
    std::string spawn_script;
    std::string next_level;
    float leaderboard_duration = 10.0f;

    std::vector<SceneEntityConfig> entities;
};
