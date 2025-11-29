/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Components.hpp
*/

#pragma once

#include <string>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Joystick.hpp>

struct Position2D {
    float x;
    float y;
};

struct Velocity2D {
    float vx;
    float vy;
};

struct Sprite2D {
    std::string texturePath;
    int rectLeft;
    int rectTop;
    int rectWidth;
    int rectHeight;
    float scale = 1.0f;
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
    sf::Joystick::Axis axisX = sf::Joystick::X;
    sf::Joystick::Axis axisY = sf::Joystick::Y;
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