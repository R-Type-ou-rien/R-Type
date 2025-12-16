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
#include "Components/tag_component.hpp"
#include "ECS/Utils/slot_map/slot_map.hpp"
#include "ISystem.hpp"
#include "InputAction.hpp"

class Registry;
struct system_context;

struct PatternComponent {
    static constexpr auto name = "Pattern";
    std::vector<std::pair<float, float>> waypoints;
    int current_index = 0;
    float speed = 100.0f;
    bool loop = false;
    bool is_active = true;
};

struct ResourceStat {
    float current;
    float max;
    float regenRate;
};

struct ResourceComponent {
    static constexpr auto name = "Resource";
    std::map<std::string, ResourceStat> resources;
    std::map<std::string, std::function<void()>> empty_effects;
};

struct transform_component_s {
    static constexpr auto name = "Transform";
    float x;
    float y;
    float scale_x = 1.0;
    float scale_y = 1.0;
    float rotation = 0;
};

struct Velocity2D {
    static constexpr auto name = "Velocity";
    float vx;
    float vy;
};

struct rect {
    static constexpr auto name = "Rect";
    float x;
    float y;
    float width;
    float height;
};

struct BoxCollisionComponent {
    static constexpr auto name = "Collision";
    CollidedEntity collision;
    std::vector<std::string> tagCollision;
    std::function<void(Registry& registry, system_context context, std::size_t current_entity)> callbackOnCollide;
};

struct sprite2D_component_s {
    static constexpr auto name = "Sprite";
    handle_t<sf::Texture> handle;
    rect dimension = {0.0f, 0.0f, 0.0f, 0.0f};
    bool is_animated = false;
    float animation_speed = 0;
    int current_animation_frame = 0;
    float last_animation_update = 0;
    int z_index = 0;
};

struct TagComponent {
    static constexpr auto name = "Tag";
    std::vector<std::string> tags;
};

using ActionCallback = std::function<void(Registry& registry, system_context context, std::size_t current_entity)>;

struct ActionScript {
    static constexpr auto name = "ActionScript";
    std::unordered_map<Action, ActionCallback> actionOnPressed;
    std::unordered_map<Action, ActionCallback> actionOnReleased;
    std::unordered_map<Action, ActionCallback> actionPressed;
};

struct Shooter {
    static constexpr auto name = "OLD SHOOTER";
    sf::Keyboard::Key shootKey;
    float projectileSpeed;
    float projectileLifetime;
    float fireRate;                 // tirs par seconde
    float timeSinceLastShot = 0.f;  // état interne, géré par le système
};

struct Projectile {
    static constexpr auto name = "Projectile";
    float lifetime;
};

struct Scroll {
    static constexpr auto name = "Scroll";
    float scroll_speed_x;
    float scroll_speed_y;
    bool is_paused;
};

struct BackgroundComponent {
    static constexpr auto name = "Background";
    handle_t<sf::Texture> texture_handle;
    float x_offset = 0.f;
    float scroll_speed = 0.f;
};