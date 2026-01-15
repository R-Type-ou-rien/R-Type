#pragma once

#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <optional>
#include <functional>
#include <set>
#include <stdexcept>
#include <sstream>
#include <SFML/Graphics/Color.hpp>
#include "boss_component.hpp"

// struct pour toute les donnees
struct EntityConfig {
    std::optional<int> hp;
    std::optional<int> damage;
    std::optional<int> projectile_damage;
    std::optional<int> score_value;
    std::optional<int> sprite_x;
    std::optional<int> sprite_y;
    std::optional<int> sprite_w;
    std::optional<int> sprite_h;
    std::optional<int> animation_frames;
    std::optional<float> projectile_scale;
    std::optional<float> speed;
    std::optional<float> fire_rate;
    std::optional<float> amplitude;
    std::optional<float> frequency;
    std::optional<float> scale;
    std::optional<float> animation_speed;
    std::optional<float> start_x;
    std::optional<float> start_y;
    std::optional<float> min_charge_time;
    std::optional<float> max_charge_time;
    std::optional<float> oscillation_amplitude;
    std::optional<float> oscillation_frequency;
    std::optional<float> attack_pattern_interval;
    std::optional<float> death_duration;
    std::optional<int> max_phases;
    std::optional<int> total_weak_points;
    std::optional<bool> can_shoot;
    std::optional<bool> shoot_at_player;
    std::optional<bool> follow_player;
    std::optional<std::string> shoot_pattern;
    std::optional<std::string> pattern;
    std::optional<std::string> sprite_path;
    std::vector<std::string> collision_tags;
    std::vector<BossSubEntityConfig> boss_sub_entities;
    std::optional<float> margin_right;
    std::optional<float> spawn_offset_x;
    std::optional<int> z_index;
    std::optional<int> tail_segment_count;
    std::optional<float> tail_sprite_width;
    std::optional<float> tail_sprite_height;
    std::optional<float> tail_scale_multiplier;
    std::optional<float> tail_spacing_ratio;
    std::optional<float> tail_sine_phase_offset;
    std::optional<float> tail_height_multiplier;
    std::optional<int> tail_hp;
    std::optional<int> tail_collision_damage;
    std::optional<float> tail_sprite_x;
    std::optional<float> tail_sprite_y;
    std::optional<int> tail_z_index;
    std::optional<std::string> tail_sprite_path;

    BossPositionConfig toBossPositionConfig() const {
        BossPositionConfig config;
        config.margin_right = margin_right.value_or(BossDefaults::Position::MARGIN_RIGHT);
        config.spawn_offset_x = spawn_offset_x.value_or(BossDefaults::Position::SPAWN_OFFSET_X);
        config.z_index = z_index.value_or(BossDefaults::Sprite::Z_INDEX);
        return config;
    }

    BossTailConfig toBossTailConfig() const {
        BossTailConfig config;
        config.segment_count = tail_segment_count.value_or(BossDefaults::Tail::SEGMENT_COUNT);
        config.sprite_width = tail_sprite_width.value_or(BossDefaults::Tail::SPRITE_WIDTH);
        config.sprite_height = tail_sprite_height.value_or(BossDefaults::Tail::SPRITE_HEIGHT);
        config.scale_multiplier = tail_scale_multiplier.value_or(BossDefaults::Tail::SCALE_MULTIPLIER);
        config.spacing_ratio = tail_spacing_ratio.value_or(BossDefaults::Tail::SPACING_RATIO);
        config.sine_phase_offset = tail_sine_phase_offset.value_or(BossDefaults::Tail::SINE_PHASE_OFFSET);
        config.height_multiplier = tail_height_multiplier.value_or(BossDefaults::Tail::HEIGHT_MULTIPLIER);
        config.hp = tail_hp.value_or(BossDefaults::Tail::HP);
        config.collision_damage = tail_collision_damage.value_or(BossDefaults::Tail::COLLISION_DAMAGE);
        config.sprite_x = tail_sprite_x.value_or(BossDefaults::Tail::SPRITE_X);
        config.sprite_y = tail_sprite_y.value_or(BossDefaults::Tail::SPRITE_Y);
        config.z_index = tail_z_index.value_or(BossDefaults::Tail::Z_INDEX);
        config.sprite_path = tail_sprite_path.value_or(BossDefaults::TAIL_SPRITE_PATH);
        return config;
    }
};

// Interface user config struct
struct UIConfig {
    // Define one visual element of the interface for exemple text
    struct Element {
        float x = 0.f;
        float y = 0.f;
        float width = 0.f;
        float height = 0.f;
        float icon_size = 0.f;
        float icon_spacing = 0.f;
        int digit_count = 0;
        int size = 24;
        std::string font;
        sf::Color color = sf::Color::White;
    };
    std::map<std::string, Element> elements;
};

// Struct for the config in game and what happened in the game
struct GameConfig {
    std::optional<float> boss_spawn_time;
    std::optional<float> wave_interval;
    std::optional<float> obstacle_start_time;
    std::optional<float> obstacle_end_time;
    std::optional<float> boss_intro_delay;
    std::optional<float> obstacle_interval;
    std::optional<float> obstacle_speed;
    std::optional<float> scroll_speed;
    std::optional<float> enemy_spawn_interval;
    std::optional<float> pod_spawn_interval;
    std::optional<float> pod_min_spawn_interval;
    std::optional<float> pod_max_spawn_interval;
    std::optional<int> min_enemies_per_wave;
    std::optional<int> max_enemies_per_wave;
    std::optional<int> obstacle_hp;
    std::optional<int> obstacle_damage;
};

// filepath to all config file
struct MasterConfig {
    std::string player_config;
    std::string enemies_config;
    std::string boss_config;
    std::string game_config;
    std::string ui_config;
    std::string resources_config;
    std::vector<std::string> levels;
};

// pase utils for clean convert text to usefull data
namespace ParsingUtils {
    // skip tab in start and end of text
    static std::string trim(const std::string& str) {
        auto first = str.find_first_not_of(" \t");

        if (std::string::npos == first) {
            return str;
        }
        auto last = str.find_last_not_of(" \t");
        return str.substr(first, (last - first + 1));
    }

    template <typename T> T parse(const std::string& value);
    template <> inline int parse<int>(const std::string& value) {
        return std::stoi(trim(value));
    }

    template <> inline float parse<float>(const std::string& value) {
        return std::stof(trim(value));
    }

    template <> inline std::string parse<std::string>(const std::string& value) {
        return trim(value);
    }

    template <> inline bool parse<bool>(const std::string& value) {
        std::string str = trim(value);
        return (str == "true" || str == "1");
    }

    template <> inline std::vector<std::string> parse<std::vector<std::string>>(const std::string& value) {
        std::vector<std::string> res;
        std::stringstream ss(trim(value));
        std::string item;

        while (std::getline(ss, item, ',')) {
            res.push_back(trim(item));
        }
        return res;
    }

    template <> inline sf::Color parse<sf::Color>(const std::string& value) {
        static const std::unordered_map<std::string, sf::Color> colors = {
            {"Red", sf::Color::Red},
            {"Green", sf::Color::Green},
            {"Blue", sf::Color::Blue},
            {"Yellow", sf::Color::Yellow},
            {"Magenta", sf::Color::Magenta},
            {"Cyan", sf::Color::Cyan},
            {"White", sf::Color::White},
            {"Black", sf::Color::Black}
        };
        auto it = colors.find(trim(value));
        return (it != colors.end()) ? it->second : sf::Color::White;
    }
}

// link word in the config file to the variable in the structure
template <typename T>
class ConfigBinder {
public:
    // Setter is for assign value to member of structure
    using Setter = std::function<void(T&, const std::string&)>;
    std::unordered_map<std::string, Setter> bindings;

    // pointe vers la variable a l'interieur de la structure 'T'
    template <typename MemberType>
    void bind(const std::string& key, std::optional<MemberType> T::* member) {
        // register a function that will be executed later when loading
        bindings[key] = [member](T& obj, const std::string& value) {
            // convert text config to type and save in object
            obj.*member = ParsingUtils::parse<MemberType>(value);
        };
    }

    // Cette fonction sert a lier un champ classique (qui n'est pas un optional).
    template <typename MemberType>
    void bind(const std::string& key, MemberType T::* member) {
        bindings[key] = [member](T& obj, const std::string& v) {
            // On convertit et on affecte directement la valeur.
            obj.*member = ParsingUtils::parse<MemberType>(v);
        };
    }

    // Cette fonction permet d'ajouter une regle de remplissage personnalisee pour des cas plus complexes.
    void bindCustom(const std::string& key, Setter setter) {
        bindings[key] = setter;
    }
};

class ConfigLoader {
private:
    // read the file and execute a function for each line found
    static void parseFile(const std::string& filepath, std::function<void(const std::string& section, const std::string& key, const std::string& value)> callback);
    static void validate(const std::string& context, const std::set<std::string>& found, const std::set<std::string>& required);

    template <typename T>
    static const ConfigBinder<T>& getBinder();

public:
    // load every type of structure from a file
    template <typename T>
    static T load(const std::string& filepath, const std::set<std::string>& required = {}) {
        T config;
        const auto& binder = getBinder<T>();
        std::set<std::string> found;

        parseFile(filepath, [&](const std::string&, const std::string& key, const std::string& value) {
            // check if the key exist in our binder
            auto it = binder.bindings.find(key);
            if (it != binder.bindings.end()) {
                it->second(config, value);
                found.insert(key);
            }
        });

        validate(filepath, found, required);
        return config;
    }

    // load a file and organize to section and return a map who associate every name of section to a data
    template <typename T>
    static std::map<std::string, T> loadMap(const std::string& filepath, const std::set<std::string>& required = {}) {
        std::map<std::string, T> configs;
        std::map<std::string, std::set<std::string>> found_per_section;
        const auto& binder = getBinder<T>();

        parseFile(filepath, [&](const std::string& section, const std::string& key, const std::string& value) {
            if (section.empty()) {
                return;
            }

            auto it = binder.bindings.find(key);
            if (it != binder.bindings.end()) {
                it->second(configs[section], value);
                found_per_section[section].insert(key);
            }
        });

        for (const auto& [sec, found] : found_per_section) {
            validate(filepath + " [" + sec + "]", found, required);
        }
        return configs;
    }

    static UIConfig loadUIConfig(const std::string& filepath);
    static MasterConfig loadMasterConfig(const std::string& filepath);

    static EntityConfig loadEntityConfig(const std::string& filepath, const std::set<std::string>& required = {}) {
        return load<EntityConfig>(filepath, required);
    }

    static std::map<std::string, EntityConfig> loadEnemiesConfig(const std::string& filepath, const std::set<std::string>& required = {}) {
        return loadMap<EntityConfig>(filepath, required);
    }

    static GameConfig loadGameConfig(const std::string& filepath, const std::set<std::string>& required = {}) {
        return load<GameConfig>(filepath, required);
    }

    static std::set<std::string> getRequiredPlayerFields() {
        return {"hp", "speed", "sprite"};
    }

    static std::set<std::string> getRequiredEnemyFields() {
        return {"hp", "speed", "sprite"};
    }

    static std::set<std::string> getRequiredBossFields() {
        return {"hp", "speed", "sprite"};
    }

    static std::set<std::string> getRequiredGameFields() {
        return {"scroll_speed"};
    }
};
