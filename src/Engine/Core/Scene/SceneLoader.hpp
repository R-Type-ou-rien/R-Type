/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** SceneLoader - Loads scene configuration from files
*/

#pragma once

#include <vector>
#include "LevelConfig.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

class SceneLoader {
   public:
    static LevelConfig loadFromFile(const std::string& filepath) {
        LevelConfig config;
        std::ifstream file(filepath);

        if (!file.is_open()) {
            throw std::runtime_error("Cannot open scene file: " + filepath);
        }

        std::string line;
        std::string current_section;

        while (std::getline(file, line)) {
            line = trim(line);

            if (line.empty() || line[0] == '#')
                continue;

            if (line[0] == '[') {
                size_t end = line.find(']');
                if (end != std::string::npos) {
                    current_section = line.substr(1, end - 1);
                }
                continue;
            }

            size_t pos = line.find('=');
            if (pos == std::string::npos)
                continue;

            std::string key = trim(line.substr(0, pos));
            std::string value = trim(line.substr(pos + 1));

            if (current_section == "LEVEL") {
                parseLevelProperty(config, key, value);
            } else if (current_section == "WALL") {
                parseWallEntry(config, value);
            } else if (current_section == "ENEMY") {
                parseEnemyEntry(config, value);
            } else if (current_section == "TURRET") {
                parseTurretEntry(config, value);
            } else if (current_section == "DECOR") {
                parseDecorEntry(config, value);
            }
        }

        return config;
    }

   private:
    static std::string trim(const std::string& str) {
        size_t start = str.find_first_not_of(" \t");
        size_t end = str.find_last_not_of(" \t");
        if (start == std::string::npos)
            return "";
        return str.substr(start, end - start + 1);
    }

    static void parseLevelProperty(LevelConfig& config, const std::string& key, const std::string& value) {
        if (key == "name")
            config.name = value;
        else if (key == "background")
            config.background_texture = value;
        else if (key == "music")
            config.music_track = value;
        else if (key == "enemies_config")
            config.enemies_config = value;
        else if (key == "boss_config")
            config.boss_config = value;
        else if (key == "boss_section")
            config.boss_section = value;
        else if (key == "game_config")
            config.game_config = value;
        else if (key == "spawn_script")
            config.spawn_script = value;
        else if (key == "next_level")
            config.next_level = value;
        else if (key == "leaderboard_duration") {
            try {
                config.leaderboard_duration = std::stof(value);
            } catch (...) {
                std::cerr << "Warning: Invalid leaderboard_duration: " << value << std::endl;
            }
        }
    }

    static void parseWallEntry(LevelConfig& config, const std::string& value) {
        SceneEntityConfig entity;
        entity.type = "Wall";

        std::stringstream ss(value);
        std::string token;
        std::vector<std::string> tokens;

        while (std::getline(ss, token, ',')) {
            tokens.push_back(trim(token));
        }

        if (tokens.size() >= 4) {
            entity.properties["x"] = std::stof(tokens[0]);
            entity.properties["y"] = std::stof(tokens[1]);
            entity.properties["width"] = std::stof(tokens[2]);
            entity.properties["height"] = std::stof(tokens[3]);
        }
        if (tokens.size() >= 5) {
            entity.properties["sprite"] = tokens[4];
        }
        if (tokens.size() >= 6) {
            entity.properties["destructible"] = (tokens[5] == "true");
        }

        config.entities.push_back(entity);
    }

    static void parseEnemyEntry(LevelConfig& config, const std::string& value) {
        SceneEntityConfig entity;
        entity.type = "Enemy";

        std::stringstream ss(value);
        std::string token;
        std::vector<std::string> tokens;

        while (std::getline(ss, token, ',')) {
            tokens.push_back(trim(token));
        }

        if (tokens.size() >= 4) {
            entity.properties["spawn_time"] = std::stof(tokens[0]);
            entity.properties["enemy_type"] = tokens[1];
            entity.properties["x"] = std::stof(tokens[2]);
            entity.properties["y"] = std::stof(tokens[3]);
        }

        config.entities.push_back(entity);
    }

    static void parseTurretEntry(LevelConfig& config, const std::string& value) {
        SceneEntityConfig entity;
        entity.type = "Turret";

        std::stringstream ss(value);
        std::string token;
        std::vector<std::string> tokens;

        while (std::getline(ss, token, ',')) {
            tokens.push_back(trim(token));
        }

        if (tokens.size() >= 2) {
            entity.properties["x"] = std::stof(tokens[0]);
            entity.properties["y"] = std::stof(tokens[1]);
        }
        if (tokens.size() >= 3) {
            entity.properties["fire_rate"] = std::stof(tokens[2]);
        }
        if (tokens.size() >= 4) {
            entity.properties["shoot_pattern"] = tokens[3];
        }
        if (tokens.size() >= 5) {
            entity.properties["sprite"] = tokens[4];
        }

        config.entities.push_back(entity);
    }

    static void parseDecorEntry(LevelConfig& config, const std::string& value) {
        SceneEntityConfig entity;
        entity.type = "Decor";

        std::stringstream ss(value);
        std::string token;
        std::vector<std::string> tokens;

        while (std::getline(ss, token, ',')) {
            tokens.push_back(trim(token));
        }

        // Format: x, y, sprite_path, scale, z_index, scroll_speed_mult
        if (tokens.size() >= 3) {
            entity.properties["x"] = std::stof(tokens[0]);
            entity.properties["y"] = std::stof(tokens[1]);
            entity.properties["sprite"] = tokens[2];
        }
        if (tokens.size() >= 4) {
            entity.properties["scale"] = std::stof(tokens[3]);
        }
        if (tokens.size() >= 5) {
            entity.properties["z_index"] = std::stoi(tokens[4]);
        }
        if (tokens.size() >= 6) {
            entity.properties["scroll_speed_mult"] = std::stof(tokens[5]);
        }

        config.entities.push_back(entity);
    }
};
