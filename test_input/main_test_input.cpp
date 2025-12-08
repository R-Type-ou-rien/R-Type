/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** main_test_input.cpp
*/


#include <iostream>
#include <optional>

#include <SFML/Graphics.hpp>

#include "../src/ecs/Input/InputAction.hpp"
#include "../src/ecs/Input/InputBinding.hpp"
#include "../src/ecs/Input/InputManager.hpp"
#include "../src/ecs/Input/InputState.hpp"

int main() {
    sf::RenderWindow window(sf::VideoMode({800u, 600u}), "InputManager Test");
    window.setFramerateLimit(60);

    // -------------------------------
    // INIT INPUT MANAGER
    // -------------------------------
    InputManager input;

    // ---- TEST BINDINGS ----
    // Clavier : espace = Action::Shoot
    input.bindAction(Action::Shoot, InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Space});

    // Clavier : Z = MoveUp
    input.bindAction(Action::MoveUp, InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Z});

    // Manette : bouton A = Shoot
    InputBinding shootPad;
    shootPad.device = InputDeviceType::GamepadButton;
    shootPad.joystickId = 0;
    shootPad.joystickButton = 0;
    input.bindAction(Action::Shoot, shootPad);

    // Gâchette droite (RT) = Shoot
    InputBinding rt;
    rt.device = InputDeviceType::GamepadAxis;
    rt.joystickId = 0;
    rt.axis = sf::Joystick::Axis::Z;
    rt.axisThreshold = 20.f;
    rt.axisSign = +1;  // direction positive
    input.bindAction(Action::Shoot, rt);

    // D-PAD haut = MoveUp
    InputBinding dpadUp;
    dpadUp.device = InputDeviceType::GamepadAxis;
    dpadUp.joystickId = 0;
    dpadUp.axis = sf::Joystick::Axis::PovY;
    dpadUp.axisThreshold = 50.f;
    dpadUp.axisSign = +1;
    input.bindAction(Action::MoveUp, dpadUp);

    // Stick gauche vers la gauche = MoveLeft
    InputBinding stickLeft;
    stickLeft.device = InputDeviceType::GamepadAxis;
    stickLeft.joystickId = 0;
    stickLeft.axis = sf::Joystick::Axis::X;  // axe horizontal du stick gauche
    stickLeft.axisThreshold = 20.f;          // deadzone
    stickLeft.axisSign = -1;                 // négatif = gauche
    input.bindAction(Action::MoveLeft, stickLeft);

    // Stick gauche vers la droite = MoveRight
    stickLeft.device = InputDeviceType::GamepadAxis;
    stickLeft.joystickId = 0;
    stickLeft.axis = sf::Joystick::Axis::X;  // axe horizontal du stick gauche
    stickLeft.axisThreshold = 20.f;          // deadzone
    stickLeft.axisSign = +1;                 // positif = droit
    input.bindAction(Action::MoveRight, stickLeft);

    // Stick gauche vers la bas = MoveDown
    stickLeft.device = InputDeviceType::GamepadAxis;
    stickLeft.joystickId = 0;
    stickLeft.axis = sf::Joystick::Axis::Y;  // axe horizontal du stick gauche
    stickLeft.axisThreshold = 20.f;          // deadzone
    stickLeft.axisSign = +1;                 // positif = haut
    input.bindAction(Action::MoveDown, stickLeft);

    // Stick gauche vers la haut = MoveUp
    stickLeft.device = InputDeviceType::GamepadAxis;
    stickLeft.joystickId = 0;
    stickLeft.axis = sf::Joystick::Axis::Y;  // axe horizontal du stick gauche
    stickLeft.axisThreshold = 20.f;          // deadzone
    stickLeft.axisSign = -1;                 // négatif = bas
    input.bindAction(Action::MoveUp, stickLeft);

    sf::Clock clock;

    std::cout << "=== InputManager Test Ready ===\n";
    std::cout << "Press SPACE or Gamepad A or RT to test Shoot\n";
    std::cout << "Press Z or D-Pad UP to test MoveUp\n";
    std::cout << "Push LEFT on left stick to test MoveLeft\n";

    while (window.isOpen()) {
        // -------------------------------
        // EVENT LOOP SFML 3
        // -------------------------------
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            if (event->is<sf::Event::FocusLost>()) {
                input.setWindowHasFocus(false);
            }
            if (event->is<sf::Event::FocusGained>()) {
                input.setWindowHasFocus(true);
            }
        }

        float dt = clock.restart().asSeconds();

        // Mise à jour des états d'actions
        input.update(dt);

        // -------------------------------
        // DEBUG PRINTS FOR ACTIONS
        // -------------------------------
        if (input.isJustPressed(Action::Shoot))
            std::cout << "[Shoot] just pressed!\n";

        if (input.isJustReleased(Action::Shoot))
            std::cout << "[Shoot] just released (hold time = " << input.getState(Action::Shoot).lastReleaseHoldTime
                      << "s)\n";

        if (input.isLongPress(Action::Shoot, 0.5f))
            std::cout << "[Shoot] LONG PRESS (>= 0.5s)\n";

        if (input.isShortPress(Action::Shoot, 0.2f))
            std::cout << "[Shoot] SHORT PRESS (< 0.2s)\n";

        if (input.isPressed(Action::MoveLeft))
            std::cout << "[MoveLeft] pressed\n";

        if (input.isPressed(Action::MoveRight))
            std::cout << "[MoveRight] pressed\n";

        if (input.isPressed(Action::MoveUp))
            std::cout << "[MoveUp] pressed\n";

        if (input.isPressed(Action::MoveDown))
            std::cout << "[MoveDown] pressed\n";

        window.clear(sf::Color::Black);
        window.display();
    }

    return 0;
}
