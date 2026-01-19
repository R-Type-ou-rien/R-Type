#include "config.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <set>

void ConfigLoader::parseFile(const std::string& filepath,
                             std::function<void(const std::string&, const std::string&, const std::string&)> callback) {
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

void ConfigLoader::validate(const std::string& context, const std::set<std::string>& found,
                            const std::set<std::string>& required) {
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
        binder.bind("oscillation_amplitude", &EntityConfig::oscillation_amplitude);
        binder.bind("oscillation_frequency", &EntityConfig::oscillation_frequency);
        binder.bind("attack_pattern_interval", &EntityConfig::attack_pattern_interval);
        binder.bind("death_duration", &EntityConfig::death_duration);
        binder.bind("max_phases", &EntityConfig::max_phases);
        binder.bind("total_weak_points", &EntityConfig::total_weak_points);
        binder.bind("can_shoot", &EntityConfig::can_shoot);
        binder.bind("shoot_at_player", &EntityConfig::shoot_at_player);
        binder.bind("follow_player", &EntityConfig::follow_player);
        binder.bind("sprite", &EntityConfig::sprite_path);
        binder.bind("shoot_pattern", &EntityConfig::shoot_pattern);
        binder.bind("pattern", &EntityConfig::pattern);
        binder.bind("boss_type", &EntityConfig::boss_type);
        binder.bind("collision_tags", &EntityConfig::collision_tags);
        binder.bind("phase1_patterns", &EntityConfig::phase1_patterns);
        binder.bind("phase2_patterns", &EntityConfig::phase2_patterns);
        binder.bind("phase3_patterns", &EntityConfig::phase3_patterns);
        binder.bind("enraged_patterns", &EntityConfig::enraged_patterns);

        binder.bind("margin_right", &EntityConfig::margin_right);
        binder.bind("spawn_offset_x", &EntityConfig::spawn_offset_x);
        binder.bind("spawn_offset_y", &EntityConfig::spawn_offset_y);
        binder.bind("z_index", &EntityConfig::z_index);

        binder.bind("tail_segment_count", &EntityConfig::tail_segment_count);
        binder.bind("tail_sprite_width", &EntityConfig::tail_sprite_width);
        binder.bind("tail_sprite_height", &EntityConfig::tail_sprite_height);
        binder.bind("tail_scale_multiplier", &EntityConfig::tail_scale_multiplier);
        binder.bind("tail_spacing_ratio", &EntityConfig::tail_spacing_ratio);
        binder.bind("tail_sine_phase_offset", &EntityConfig::tail_sine_phase_offset);
        binder.bind("tail_height_multiplier", &EntityConfig::tail_height_multiplier);
        binder.bind("tail_hp", &EntityConfig::tail_hp);
        binder.bind("tail_collision_damage", &EntityConfig::tail_collision_damage);
        binder.bind("tail_sprite_x", &EntityConfig::tail_sprite_x);
        binder.bind("tail_sprite_y", &EntityConfig::tail_sprite_y);
        binder.bind("tail_z_index", &EntityConfig::tail_z_index);
        binder.bind("tail_sprite_path", &EntityConfig::tail_sprite_path);

        binder.bind("projectile_sprite", &EntityConfig::projectile_sprite);
        binder.bind("projectile_sprite_x", &EntityConfig::projectile_sprite_x);
        binder.bind("projectile_sprite_y", &EntityConfig::projectile_sprite_y);
        binder.bind("projectile_sprite_w", &EntityConfig::projectile_sprite_w);
        binder.bind("projectile_sprite_h", &EntityConfig::projectile_sprite_h);

        binder.bind("charged_sprite", &EntityConfig::charged_sprite);
        binder.bind("charged_sprite_x", &EntityConfig::charged_sprite_x);
        binder.bind("charged_sprite_y", &EntityConfig::charged_sprite_y);
        binder.bind("charged_sprite_w", &EntityConfig::charged_sprite_w);
        binder.bind("charged_sprite_h", &EntityConfig::charged_sprite_h);

        binder.bindCustom("sub_entity", [](EntityConfig& cfg, const std::string& v) {
            std::vector<std::string> parts = ParsingUtils::parse<std::vector<std::string>>(v);
            if (parts.size() >= 4) {
                BossSubEntityConfig sub;
                sub.type = parts[0];
                sub.offset_x = std::stof(parts[1]);
                sub.offset_y = std::stof(parts[2]);
                sub.fire_rate = std::stof(parts[3]);
                cfg.boss_sub_entities.push_back(sub);
            }
        });
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
        binder.bindCustom("levels",
                          [](MasterConfig& cfg, const std::string& v) { cfg.levels.push_back(ParsingUtils::trim(v)); });
    }
    return binder;
}

UIConfig ConfigLoader::loadUIConfig(const std::string& filepath) {
    UIConfig config;
    const auto& binder = getBinder<UIConfig::Element>();

    parseFile(filepath, [&](const std::string& section, const std::string& key, const std::string& value) {
        if (section.empty()) {
            return;
        }

        auto it = binder.bindings.find(key);

        if (it != binder.bindings.end()) {
            it->second(config.elements[section], value);
        }
    });
    return config;
}

MasterConfig ConfigLoader::loadMasterConfig(const std::string& filepath) {
    return load<MasterConfig>(filepath);
}
