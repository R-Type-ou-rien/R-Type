 

#pragma once

#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

enum class InputDeviceType {
    Keyboard,
    MouseButton,
    GamepadButton,
    GamepadAxis,
};

struct InputBinding {
    InputDeviceType device = InputDeviceType::Keyboard;

    
    sf::Keyboard::Key key = sf::Keyboard::Key::Unknown;

    
    sf::Mouse::Button mouseButton = sf::Mouse::Button::Left;

    
    unsigned int joystickId = 0;
    unsigned int joystickButton = 0;

    
    sf::Joystick::Axis axis = sf::Joystick::Axis::X;
    float axisThreshold = 50.f;
    int axisSign = +1;
};

 
