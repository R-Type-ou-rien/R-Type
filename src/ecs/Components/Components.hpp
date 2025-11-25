/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Components.hpp
*/

#pragma once

#include <string>
#include <SFML/Window/Keyboard.hpp>

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