 

#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Keyboard.hpp>

#include "../../Engine/Core/ECS/Utils/slot_map/slot_map.hpp"

struct transform_component_s {
    float x;
    float y;
    sf::Vector2f scale = {1.0, 1.0};
    float rotation = 0;
};

struct Velocity2D {
    float vx;
    float vy;
};

struct TypeEntityComponent {
    enum TypeEntity { PLAYER, ENEMY_BASIC, ENEMY_SHOOTER };
    TypeEntity type;
};

struct sprite2D_component_s {
    handle_t<sf::Texture> handle;
    sf::IntRect dimension;
    float animation_speed;
    int current_animation_frame = 0;
    float last_animation_update = 0;
    int z_index = 0;
};

struct InputControl {
    sf::Keyboard::Key up;
    sf::Keyboard::Key down;
    sf::Keyboard::Key left;
    sf::Keyboard::Key right;
    float speed;
};

struct GamepadControl {
    unsigned int joystickId = 0;  
    sf::Joystick::Axis axisX = sf::Joystick::Axis::X;
    sf::Joystick::Axis axisY = sf::Joystick::Axis::Y;
    unsigned int buttonShoot = 0;  
    float speed = 300.f;           
    float deadZone = 15.f;         
};

struct Shooter {
    sf::Keyboard::Key shootKey;
    float projectileSpeed;
    float projectileLifetime;
    float fireRate;                 
    float timeSinceLastShot = 0.f;  
};

struct Projectile {
    float lifetime;
};

struct Scroll {
    float scroll_speed_x;
    float scroll_speed_y;
    bool is_paused;
};
