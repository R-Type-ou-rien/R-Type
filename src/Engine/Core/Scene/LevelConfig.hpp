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
    std::vector<SceneEntityConfig> entities;
};
