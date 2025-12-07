/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Components.hpp
*/

#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Joystick.hpp>
#include "../../utils/slot_map/slot_map.hpp"

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
    unsigned int joystickId = 0;              // id de la manette (0 par défaut)
    sf::Joystick::Axis axisX = sf::Joystick::Axis::X;
    sf::Joystick::Axis axisY = sf::Joystick::Axis::Y;
    unsigned int buttonShoot = 0;             // pour plus tard (tir)
    float speed = 300.f;                      // vitesse max
    float deadZone = 15.f;                    // pour ignorer les petits mouvements
};

struct Shooter {
    sf::Keyboard::Key shootKey;
    float projectileSpeed;
    float projectileLifetime;
    float fireRate;           // tirs par seconde
    float timeSinceLastShot = 0.f; // état interne, géré par le système
};

struct Projectile {
    float lifetime;
};

struct Scroll {
    float scroll_speed_x;
    float scroll_speed_y;
    bool is_paused;
};