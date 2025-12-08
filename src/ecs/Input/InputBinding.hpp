/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** InputBinding.hpp
*/

#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Joystick.hpp>

enum class InputDeviceType {
    Keyboard,
    MouseButton,
    GamepadButton,
    GamepadAxis,
};

struct InputBinding {
    InputDeviceType device = InputDeviceType::Keyboard;

    // Keyboard
    sf::Keyboard::Key key = sf::Keyboard::Key::Unknown;

    // Mouse
    sf::Mouse::Button mouseButton = sf::Mouse::Button::Left;

    // Gamepad button
    unsigned int joystickId = 0;
    unsigned int joystickButton = 0;

    // Gamepad axis (joy stick / gachette) ---
    sf::Joystick::Axis axis = sf::Joystick::Axis::X;
    float axisThreshold = 50.f;
    int axisSign = +1;
};

/*

Exemple 

// Tir au clavier : J
InputBinding kbShoot{
    InputDeviceType::Keyboard,
    sf::Keyboard::Key::J
};

// Tir manette : bouton 0 (A)
InputBinding padShootButton{
    InputDeviceType::GamepadButton,
    sf::Keyboard::Key::Unknown, // ignoré
    sf::Mouse::Button::Left,    // ignoré
    0,                          // joystickId
    0                           // joystickButton
};

// Tir gâchette droite sur axe Z
InputBinding padShootTrigger{
    InputDeviceType::GamepadAxis,
    sf::Keyboard::Key::Unknown,
    sf::Mouse::Button::Left,
    0,                            // joystickId
    0,                            // button ignoré
    sf::Joystick::Axis::Z,        // D-axis
    20.f,                         // seuil
    +1                            // > threshold
};
*/