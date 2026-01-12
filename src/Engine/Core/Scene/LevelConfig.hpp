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
    std::string game_config;
    std::string spawn_script;

    std::vector<SceneEntityConfig> entities;
};
