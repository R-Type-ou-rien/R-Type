#pragma once

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <set>
#include <optional>
#include <SFML/Graphics/Color.hpp>

// Helper pour parser les couleurs SFML
inline sf::Color parseColor(const std::string& colorName) {
    if (colorName == "Red") return sf::Color::Red;
    if (colorName == "Green") return sf::Color::Green;
    if (colorName == "Blue") return sf::Color::Blue;
    if (colorName == "Yellow") return sf::Color::Yellow;
    if (colorName == "Magenta") return sf::Color::Magenta;
    if (colorName == "Cyan") return sf::Color::Cyan;
    if (colorName == "White") return sf::Color::White;
    if (colorName == "Black") return sf::Color::Black;
    return sf::Color::White;
}

// Structure globale pour les stats d'une entité
struct EntityConfig {
    std::optional<int> hp;
    std::optional<int> damage;
    std::optional<int> projectile_damage;
    std::optional<float> projectile_scale;
    std::optional<float> speed;
    std::optional<float> fire_rate;
    std::optional<bool> can_shoot;
    std::optional<bool> shoot_at_player;
    std::optional<std::string> shoot_pattern;
    std::optional<bool> follow_player;
    std::optional<std::string> pattern;
    std::optional<float> amplitude;
    std::optional<float> frequency;
    std::optional<std::string> sprite_path;
    std::optional<int> sprite_x;
    std::optional<int> sprite_y;
    std::optional<int> sprite_w;
    std::optional<int> sprite_h;
    std::optional<float> scale;
    std::optional<int> animation_frames;
    std::optional<float> animation_speed;
    std::optional<int> score_value;
    std::optional<float> start_x;
    std::optional<float> start_y;
    std::vector<std::string> collision_tags;
    std::optional<float> min_charge_time;
    std::optional<float> max_charge_time;
};

struct UIConfig {
    struct Element {
        float x = 0, y = 0;
        int size = 24;
        std::string font;
        sf::Color color = sf::Color::White;
        float width = 0, height = 0;
        float icon_size = 0, icon_spacing = 0;
        int digit_count = 0;
    };
    std::map<std::string, Element> elements;
};

struct GameConfig {
    std::optional<float> boss_spawn_time;
    std::optional<float> wave_interval;
    std::optional<int> min_enemies_per_wave;
    std::optional<int> max_enemies_per_wave;
    std::optional<float> obstacle_start_time;
    std::optional<float> obstacle_end_time;
    std::optional<float> boss_intro_delay;
    std::optional<float> obstacle_interval;
    std::optional<float> obstacle_speed;
    std::optional<int> obstacle_hp;
    std::optional<int> obstacle_damage;
    std::optional<float> scroll_speed;
    std::optional<float> enemy_spawn_interval;
    std::optional<float> pod_spawn_interval;
    std::optional<float> pod_min_spawn_interval;
    std::optional<float> pod_max_spawn_interval;
};

class ConfigLoader {
   public:
    static EntityConfig loadEntityConfig(const std::string& filepath, const std::set<std::string>& required_fields) {
        EntityConfig config;
        std::ifstream file(filepath);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open config file: " + filepath);
        }

        std::set<std::string> found_fields;
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#')
                continue;

            size_t pos = line.find('=');
            if (pos == std::string::npos)
                continue;

            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            found_fields.insert(key);
            parseEntityValue(config, key, value);
        }

        // Vérifier que tous les champs requis sont présents
        std::string missing_fields;
        for (const auto& field : required_fields) {
            if (found_fields.find(field) == found_fields.end()) {
                if (!missing_fields.empty())
                    missing_fields += ", ";
                missing_fields += field;
            }
        }

        if (!missing_fields.empty()) {
            throw std::runtime_error("Missing required fields in " + filepath + ": " + missing_fields);
        }

        return config;
    }

    static std::map<std::string, EntityConfig> loadEnemiesConfig(const std::string& filepath,
                                                                 const std::set<std::string>& required_fields) {
        std::map<std::string, EntityConfig> configs;
        std::map<std::string, std::set<std::string>> found_fields_per_section;
        std::ifstream file(filepath);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open config file: " + filepath);
        }

        std::string line;
        std::string current_section;
        EntityConfig current_config;

        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#')
                continue;

            if (line[0] == '[') {
                if (!current_section.empty()) {
                    configs[current_section] = current_config;
                }
                size_t end = line.find(']');
                current_section = line.substr(1, end - 1);
                current_config = EntityConfig();
                found_fields_per_section[current_section] = {};
                continue;
            }

            size_t pos = line.find('=');
            if (pos == std::string::npos)
                continue;

            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            if (!current_section.empty()) {
                found_fields_per_section[current_section].insert(key);
            }
            parseEntityValue(current_config, key, value);
        }

        if (!current_section.empty()) {
            configs[current_section] = current_config;
        }

        // Vérifier les champs requis pour chaque section
        for (const auto& [section, found_fields] : found_fields_per_section) {
            std::string missing_fields;
            for (const auto& field : required_fields) {
                if (found_fields.find(field) == found_fields.end()) {
                    if (!missing_fields.empty())
                        missing_fields += ", ";
                    missing_fields += field;
                }
            }
            if (!missing_fields.empty()) {
                throw std::runtime_error("Missing required fields in [" + section + "] section of " + filepath + ": " +
                                         missing_fields);
            }
        }

        return configs;
    }

    static GameConfig loadGameConfig(const std::string& filepath, const std::set<std::string>& required_fields) {
        GameConfig config;
        std::ifstream file(filepath);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open config file: " + filepath);
        }

        std::set<std::string> found_fields;
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#')
                continue;

            size_t pos = line.find('=');
            if (pos == std::string::npos)
                continue;

            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            found_fields.insert(key);
            if (key == "boss_spawn_time")
                config.boss_spawn_time = std::stof(value);
            else if (key == "wave_interval")
                config.wave_interval = std::stof(value);
            else if (key == "min_enemies_per_wave")
                config.min_enemies_per_wave = std::stoi(value);
            else if (key == "max_enemies_per_wave")
                config.max_enemies_per_wave = std::stoi(value);
            else if (key == "obstacle_start_time")
                config.obstacle_start_time = std::stof(value);
            else if (key == "obstacle_end_time")
                config.obstacle_end_time = std::stof(value);
            else if (key == "boss_intro_delay")
                config.boss_intro_delay = std::stof(value);
            else if (key == "obstacle_interval")
                config.obstacle_interval = std::stof(value);
            else if (key == "obstacle_speed")
                config.obstacle_speed = std::stof(value);
            else if (key == "obstacle_hp")
                config.obstacle_hp = std::stoi(value);
            else if (key == "obstacle_damage")
                config.obstacle_damage = std::stoi(value);
            else if (key == "scroll_speed")
                config.scroll_speed = std::stof(value);
            else if (key == "enemy_spawn_interval")
                config.enemy_spawn_interval = std::stof(value);
            else if (key == "pod_spawn_interval")
                config.pod_spawn_interval = std::stof(value);
            else if (key == "pod_min_spawn_interval")
                config.pod_min_spawn_interval = std::stof(value);
            else if (key == "pod_max_spawn_interval")
                config.pod_max_spawn_interval = std::stof(value);
        }

        // Vérifier que tous les champs requis sont présents
        std::string missing_fields;
        for (const auto& field : required_fields) {
            if (found_fields.find(field) == found_fields.end()) {
                if (!missing_fields.empty())
                    missing_fields += ", ";
                missing_fields += field;
            }
        }

        if (!missing_fields.empty()) {
            throw std::runtime_error("Missing required fields in " + filepath + ": " + missing_fields);
        }

        return config;
    }

    static UIConfig loadUIConfig(const std::string& filepath) {
        UIConfig config;
        std::ifstream file(filepath);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open config file: " + filepath);
        }

        std::string line;
        std::string current_section;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#')
                continue;

            if (line[0] == '[') {
                size_t end = line.find(']');
                current_section = line.substr(1, end - 1);
                continue;
            }

            size_t pos = line.find('=');
            if (pos == std::string::npos)
                continue;

            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            if (current_section.empty()) continue;

            auto& element = config.elements[current_section];
            if (key == "x") element.x = std::stof(value);
            else if (key == "y") element.y = std::stof(value);
            else if (key == "size" || key == "main_size" || key == "score_size" || key == "sub_size") {
                // Pour simplifier on stocke le dernier lu ou on pourrait avoir plusieurs champs
                element.size = std::stoi(value);
            }
            else if (key == "font") element.font = value;
            else if (key == "color") element.color = parseColor(value);
            else if (key == "width") element.width = std::stof(value);
            else if (key == "height") element.height = std::stof(value);
            else if (key == "icon_size") element.icon_size = std::stof(value);
            else if (key == "icon_spacing") element.icon_spacing = std::stof(value);
            else if (key == "digit_count") element.digit_count = std::stoi(value);
        }
        return config;
    }

   private:
    static void parseEntityValue(EntityConfig& config, const std::string& key, const std::string& value) {
        if (key == "hp" || key == "max_hp")
            config.hp = std::stoi(value);
        else if (key == "damage")
            config.damage = std::stoi(value);
        else if (key == "projectile_damage")
            config.projectile_damage = std::stoi(value);
        else if (key == "projectile_scale")
            config.projectile_scale = std::stof(value);
        else if (key == "speed")
            config.speed = std::stof(value);
        else if (key == "fire_rate")
            config.fire_rate = std::stof(value);
        else if (key == "can_shoot")
            config.can_shoot = (value == "true");
        else if (key == "shoot_at_player")
            config.shoot_at_player = (value == "true");
        else if (key == "shoot_pattern")
            config.shoot_pattern = value;  // Nouveau
        else if (key == "follow_player")
            config.follow_player = (value == "true");
        else if (key == "pattern")
            config.pattern = value;
        else if (key == "amplitude")
            config.amplitude = std::stof(value);
        else if (key == "frequency")
            config.frequency = std::stof(value);
        else if (key == "sprite")
            config.sprite_path = value;
        else if (key == "sprite_x")
            config.sprite_x = std::stoi(value);
        else if (key == "sprite_y")
            config.sprite_y = std::stoi(value);
        else if (key == "sprite_w")
            config.sprite_w = std::stoi(value);
        else if (key == "sprite_h")
            config.sprite_h = std::stoi(value);
        else if (key == "score_value")
            config.score_value = std::stoi(value);
        else if (key == "start_x")
            config.start_x = std::stof(value);
        else if (key == "start_y")
            config.start_y = std::stof(value);
        else if (key == "scale")
            config.scale = std::stof(value);
        else if (key == "animation_frames")
            config.animation_frames = std::stoi(value);
        else if (key == "animation_speed")
            config.animation_speed = std::stof(value);
        else if (key == "min_charge_time")
            config.min_charge_time = std::stof(value);
        else if (key == "max_charge_time")
            config.max_charge_time = std::stof(value);
        else if (key == "collision_tags") {
            std::stringstream ss(value);
            std::string tag;
            while (std::getline(ss, tag, ',')) {
                config.collision_tags.push_back(tag);
            }
        }
    }

   public:
    static const std::set<std::string>& getRequiredPlayerFields() {
        static const std::set<std::string> fields = {"hp",       "damage",   "speed",    "fire_rate",
                                                     "sprite",   "sprite_x", "sprite_y", "sprite_w",
                                                     "sprite_h", "start_x",  "start_y"};
        return fields;
    }

    static const std::set<std::string>& getRequiredEnemyFields() {
        static const std::set<std::string> fields = {"hp",       "damage",   "speed",    "sprite",     "sprite_x",
                                                     "sprite_y", "sprite_w", "sprite_h", "score_value"};
        return fields;
    }

    static const std::set<std::string>& getRequiredBossFields() {
        static const std::set<std::string> fields = {"hp",       "damage",      "speed",    "fire_rate",
                                                     "sprite",   "sprite_x",    "sprite_y", "sprite_w",
                                                     "sprite_h", "score_value", "can_shoot"};
        return fields;
    }

    static const std::set<std::string>& getRequiredGameFields() {
        static const std::set<std::string> fields = {
            "boss_spawn_time",     "wave_interval",     "min_enemies_per_wave", "max_enemies_per_wave",
            "obstacle_start_time", "obstacle_end_time", "boss_intro_delay"};
        return fields;
    }
};
