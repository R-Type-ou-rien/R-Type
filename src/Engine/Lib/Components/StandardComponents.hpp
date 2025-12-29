/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Components.hpp
*/

#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/String.hpp>
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <functional>
#include <map>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Components/tag_component.hpp"
#include "ECS/Utils/slot_map/slot_map.hpp"
#include "ISystem.hpp"
#include "InputAction.hpp"
#include "registry.hpp"

struct PatternComponent {
    static constexpr std::string_view name = "PatternComponent";
    enum PatternType { WAYPOINT, STRAIGHT, SINUSOIDAL };
    PatternType type = WAYPOINT;
    std::vector<std::pair<float, float>> waypoints;
    int current_index = 0;
    float speed = 100.0f;
    bool loop = false;
    bool is_active = true;
    float amplitude = 50.0f;
    float frequency = 2.0f;
    float time_elapsed = 0.0f;
};

struct ResourceStat {
    float current;
    float max;
    float regenRate;
};

struct ResourceComponent {
    static constexpr std::string_view name = "ResourceComponent";
    std::map<std::string, ResourceStat> resources;
    std::map<std::string, std::function<void()>> empty_effects;
};

struct transform_component_s {
    static constexpr std::string_view name = "TransformComponent";
    float x;
    float y;
    float scale_x = 1.0;
    float scale_y = 1.0;
    float rotation = 0;
};

struct Velocity2D {
    static constexpr std::string_view name = "Velocity2DComponent";
    float vx;
    float vy;
};

struct rect {
    float x;
    float y;
    float width;
    float height;
};

struct BoxCollisionComponent {
    static constexpr std::string_view name = "CollisionComponent";
    CollidedEntity collision;
    std::vector<std::string> tagCollision;
    std::function<void(Registry& registry, system_context context, Entity current_entity)> callbackOnCollide;
};

struct sprite2D_component_s {
    static constexpr std::string_view name = "Sprite2DComponent";
    handle_t<sf::Texture> handle;
    rect dimension = {0.0f, 0.0f, 0.0f, 0.0f};
    bool is_animated = false;
    std::vector<rect> frames;
    bool reverse_animation = false;
    bool loop_animation = false;
    float animation_speed = 0;
    int current_animation_frame = 0;
    float last_animation_update = 0;
    int z_index = 0;
    float lastUpdateTime = 0.f;
};

struct TagComponent {
    static constexpr std::string_view name = "TagComponent";
    std::vector<std::string> tags;
};

struct TextComponent {
    static constexpr std::string_view name = "TextComponent";
    std::string text;
    std::string fontPath;
    unsigned int characterSize = 24;
    sf::Color color = sf::Color::White;
    float x;
    float y;
};

using ActionCallback = std::function<void(Registry& registry, system_context context, Entity current_entity)>;

struct ActionScript {
    static constexpr std::string_view name = "ActionEffectComponent";
    std::unordered_map<Action, ActionCallback> actionOnPressed;
    std::unordered_map<Action, ActionCallback> actionOnReleased;
    std::unordered_map<Action, ActionCallback> actionPressed;
};

struct Shooter {
    static constexpr std::string_view name = "ShooterComponent";
    sf::Keyboard::Key shootKey;
    float projectileSpeed;
    float projectileLifetime;
    float fireRate;                 // tirs par seconde
    float timeSinceLastShot = 0.f;  // état interne, géré par le système
};

struct Projectile {
    static constexpr std::string_view name = "ProjectileComponent";
    float lifetime;
};

struct Scroll {
    static constexpr std::string_view name = "ScrollComponent";
    float scroll_speed_x;
    float scroll_speed_y;
    bool is_paused;
};

struct BackgroundComponent {
    static constexpr std::string_view name = "BackgroundComponent";
    handle_t<sf::Texture> texture_handle;
    float x_offset = 0.f;
    float scroll_speed = 0.f;
};

// struct SpawnComponent {
//     bool active = true;
//     float interval = 2.0f;
//     float elapsed = 0.0f;

//     std::string sprite_path = "content/sprites/r-typesheet8.gif";
//     rect frame {0, 0, 32, 32};

//     float scale_x = 2.0f;
//     float scale_y = 2.0f;
//     float speed_x = -100.0f;
//     bool allow_outside_right = true;
// };
