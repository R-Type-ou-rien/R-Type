// /*
// ** EPITECH PROJECT, 2025
// ** R-Type
// ** File description:
// ** InputManager.hpp
// */

// #pragma once

// #include <unordered_map>
// #include <vector>

// #include "ActionRegistry.hpp"
// #include "InputAction.hpp"
// #include "InputBinding.hpp"
// #include "InputState.hpp"

// class InputManager {
//    public:
//     void bindAction(Action action, const InputBinding& binding);

//     void update(float dt);

//     void setWindowHasFocus(bool focus) { _hasFocus = focus; }

//     bool isPressed(Action action) const;
//     bool isJustPressed(Action action) const;
//     bool isJustReleased(Action action) const;
//     bool isShortPress(Action action, float threshold) const;
//     bool isLongPress(Action action, float threshold) const;
//     const ActionState& getState(Action action) const;

//    private:
//     ActionRegistry _actionRegistry;

//     bool isBindingActive(const InputBinding& binding) const;

//     std::unordered_map<Action, std::vector<InputBinding>> _bindings;
//     std::unordered_map<Action, ActionState> _states;
//     bool _hasFocus = true;
// };

// /*

// Exemple d'utilisation dans le main

// InputManager input;

// // Déplacements clavier
// input.bindAction("move_up",    { InputDeviceType::Keyboard, sf::Keyboard::Key::Z });
// input.bindAction("move_down",  { InputDeviceType::Keyboard, sf::Keyboard::Key::S });
// input.bindAction("move_left",  { InputDeviceType::Keyboard, sf::Keyboard::Key::Q });
// input.bindAction("move_right", { InputDeviceType::Keyboard, sf::Keyboard::Key::D });

// // Tir clavier
// input.bindAction(Action::Shoot, { InputDeviceType::Keyboard, sf::Keyboard::Key::J });

// // Tir manette : bouton A (0) et gâchette droite sur Z
// InputBinding shootButton;
// shootButton.device = InputDeviceType::GamepadButton;
// shootButton.joystickId = 0;
// shootButton.joystickButton = 0;
// input.bindAction(Action::Shoot, shootButton);

// InputBinding shootTrigger;
// shootTrigger.device = InputDeviceType::GamepadAxis;
// shootTrigger.joystickId = 0;
// shootTrigger.axis = sf::Joystick::Axis::Z;
// shootTrigger.axisThreshold = 20.f;
// shootTrigger.axisSign = +1;
// input.bindAction(Action::Shoot, shootTrigger);

// ----------------------------------------------------------

// sf::Clock clock;

// while (window.isOpen()) {
//     sf::Event event;
//     while (window.pollEvent(event)) {
//         if (event.type == sf::Event::Closed)
//             window.close();

//         if (event.type == sf::Event::LostFocus)
//             input.setWindowHasFocus(false);
//         if (event.type == sf::Event::GainedFocus)
//             input.setWindowHasFocus(true);
//     }

//     float dt = clock.restart().asSeconds();

//     input.update(dt);

//     // Exemple d’utilisation :
//     if (input.isPressed("move_up")) {
//         // velocity.y -= speed;
//     }
//     if (input.isPressed("move_left")) {
//         // velocity.x -= speed;
//     }

//     // Tir auto (comme tu fais déjà)
//     if (input.isPressed(Action::Shoot)) {
//         // Ton ShootSystem peut utiliser ça au lieu de Keyboard::isKeyPressed
//     }

//     // Tir spécial sur appui court
//     if (input.isShortPress(Action::Shoot, 0.2f)) {
//         // petit tir, tap court
//     }

//     // Tir chargé sur appui long
//     if (input.isLongPress(Action::Shoot, 0.5f)) {
//         // charge shot
//     }

//     // update ECS, render, etc.
// }

// */
