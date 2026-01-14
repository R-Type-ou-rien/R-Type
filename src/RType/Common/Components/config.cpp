#include "config.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

void ConfigLoader::parseFile(const std::string& filepath, std::function<void(const std::string&, const std::string&, const std::string&)> callback) {
    std::ifstream file(filepath);

    if (!file.is_open()) {
        throw std::runtime_error("Cannot open config: " + filepath);
    }

    std::string line;
    std::string section;

    while (std::getline(file, line)) {
        size_t comment = line.find('#');

        // ignore the comment line
        if (comment != std::string::npos) {
            line = line.substr(0, comment);
        }
        
        std::string clean = ParsingUtils::trim(line);

        if (clean.empty()) {
            continue;
        }

        // check if its the section like [boss] and extract the name
        if (clean.front() == '[' && clean.back() == ']') {
            section = clean.substr(1, clean.size() - 2);
            continue;
        }

        size_t eq = clean.find('=');

        if (eq != std::string::npos) {
            callback(section, ParsingUtils::trim(clean.substr(0, eq)), ParsingUtils::trim(clean.substr(eq + 1)));
        }
    }
}

void ConfigLoader::validate(const std::string& context, const std::set<std::string>& found, const std::set<std::string>& required) {
    std::string missing;

    for (const auto& field : required) {
        if (found.find(field) == found.end()) {
            missing += field + ", ";
        }
    }

    if (!missing.empty()) {
        throw std::runtime_error("Missing fields in " + context + ": " + missing);
    }
}

template <>
const ConfigBinder<EntityConfig>& ConfigLoader::getBinder<EntityConfig>() {
    static ConfigBinder<EntityConfig> binder;
    
    if (binder.bindings.empty()) {
        binder.bind("hp", &EntityConfig::hp);
        binder.bind("max_hp", &EntityConfig::hp);
        binder.bind("damage", &EntityConfig::damage);
        binder.bind("projectile_damage", &EntityConfig::projectile_damage);
        binder.bind("score_value", &EntityConfig::score_value);
        binder.bind("sprite_x", &EntityConfig::sprite_x);
        binder.bind("sprite_y", &EntityConfig::sprite_y);
        binder.bind("sprite_w", &EntityConfig::sprite_w);
        binder.bind("sprite_h", &EntityConfig::sprite_h);
        binder.bind("animation_frames", &EntityConfig::animation_frames);
        binder.bind("speed", &EntityConfig::speed);
        binder.bind("fire_rate", &EntityConfig::fire_rate);
        binder.bind("projectile_scale", &EntityConfig::projectile_scale);
        binder.bind("amplitude", &EntityConfig::amplitude);
        binder.bind("frequency", &EntityConfig::frequency);
        binder.bind("scale", &EntityConfig::scale);
        binder.bind("animation_speed", &EntityConfig::animation_speed);
        binder.bind("start_x", &EntityConfig::start_x);
        binder.bind("start_y", &EntityConfig::start_y);
        binder.bind("min_charge_time", &EntityConfig::min_charge_time);
        binder.bind("max_charge_time", &EntityConfig::max_charge_time);
        binder.bind("can_shoot", &EntityConfig::can_shoot);
        binder.bind("shoot_at_player", &EntityConfig::shoot_at_player);
        binder.bind("follow_player", &EntityConfig::follow_player);
        binder.bind("sprite", &EntityConfig::sprite_path);
        binder.bind("shoot_pattern", &EntityConfig::shoot_pattern);
        binder.bind("pattern", &EntityConfig::pattern);
        binder.bind("collision_tags", &EntityConfig::collision_tags);
    }
    return binder;
}

template <>
const ConfigBinder<GameConfig>& ConfigLoader::getBinder<GameConfig>() {
    static ConfigBinder<GameConfig> binder;

    if (binder.bindings.empty()) {
        binder.bind("boss_spawn_time", &GameConfig::boss_spawn_time);
        binder.bind("wave_interval", &GameConfig::wave_interval);
        binder.bind("obstacle_start_time", &GameConfig::obstacle_start_time);
        binder.bind("obstacle_end_time", &GameConfig::obstacle_end_time);
        binder.bind("boss_intro_delay", &GameConfig::boss_intro_delay);
        binder.bind("obstacle_interval", &GameConfig::obstacle_interval);
        binder.bind("enemy_spawn_interval", &GameConfig::enemy_spawn_interval);
        binder.bind("pod_spawn_interval", &GameConfig::pod_spawn_interval);
        binder.bind("pod_min_spawn_interval", &GameConfig::pod_min_spawn_interval);
        binder.bind("pod_max_spawn_interval", &GameConfig::pod_max_spawn_interval);
        binder.bind("min_enemies_per_wave", &GameConfig::min_enemies_per_wave);
        binder.bind("max_enemies_per_wave", &GameConfig::max_enemies_per_wave);
        binder.bind("obstacle_hp", &GameConfig::obstacle_hp);
        binder.bind("obstacle_damage", &GameConfig::obstacle_damage);
        binder.bind("obstacle_speed", &GameConfig::obstacle_speed);
        binder.bind("scroll_speed", &GameConfig::scroll_speed);
    }
    return binder;
}

template <>
const ConfigBinder<UIConfig::Element>& ConfigLoader::getBinder<UIConfig::Element>() {
    static ConfigBinder<UIConfig::Element> binder;

    if (binder.bindings.empty()) {
        binder.bind("x", &UIConfig::Element::x);
        binder.bind("y", &UIConfig::Element::y);
        binder.bind("width", &UIConfig::Element::width);
        binder.bind("height", &UIConfig::Element::height);
        binder.bind("size", &UIConfig::Element::size);
        binder.bind("digit_count", &UIConfig::Element::digit_count);
        binder.bind("icon_size", &UIConfig::Element::icon_size);
        binder.bind("icon_spacing", &UIConfig::Element::icon_spacing);
        binder.bind("font", &UIConfig::Element::font);
        binder.bind("color", &UIConfig::Element::color); 
    }
    return binder;
}

template <>
const ConfigBinder<MasterConfig>& ConfigLoader::getBinder<MasterConfig>() {
    static ConfigBinder<MasterConfig> binder;

    if (binder.bindings.empty()) {
        binder.bind("player", &MasterConfig::player_config);
        binder.bind("enemies", &MasterConfig::enemies_config);
        binder.bind("boss", &MasterConfig::boss_config);
        binder.bind("game", &MasterConfig::game_config);
        binder.bind("ui", &MasterConfig::ui_config);
        binder.bind("resources", &MasterConfig::resources_config);        
        binder.bindCustom("level", [](MasterConfig& cfg, const std::string& v) {
            cfg.levels.push_back(ParsingUtils::trim(v));
        });
    }
    return binder;
}

UIConfig ConfigLoader::loadUIConfig(const std::string& filepath) {
    UIConfig config;
    const auto& binder = getBinder<UIConfig::Element>();

    // parse when its a section for UI
    parseFile(filepath, [&](const std::string& section, const std::string& key, const std::string& value) {
        if (section.empty()) {
            return;
        }

        auto it = binder.bindings.find(key);

        if (it != binder.bindings.end()) {
            // get the specific element of the actual section with the value found
            it->second(config.elements[section], value);
        }
    });
    return config;
}

MasterConfig ConfigLoader::loadMasterConfig(const std::string& filepath) {
    return load<MasterConfig>(filepath);
}
