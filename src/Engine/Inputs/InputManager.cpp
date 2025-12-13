/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** InputManager.cpp
*/

#include "InputManager.hpp"


void InputManager::bindAction(Action action, const InputBinding& binding) {
    _actionRegistry.registerAction(action);
    _bindings[action].push_back(binding);
    // _states.try_emplace(action, ActionState{});
}

bool InputManager::isBindingActive(const InputBinding& b) const {
    switch (b.device) {
        case InputDeviceType::Keyboard:
            return sf::Keyboard::isKeyPressed(b.key);

        case InputDeviceType::MouseButton:
            return sf::Mouse::isButtonPressed(b.mouseButton);

        case InputDeviceType::GamepadButton:
            if (!sf::Joystick::isConnected(b.joystickId))
                return false;
            return sf::Joystick::isButtonPressed(b.joystickId, b.joystickButton);

        case InputDeviceType::GamepadAxis:
            if (!sf::Joystick::isConnected(b.joystickId))
                return false;
            if (!sf::Joystick::hasAxis(b.joystickId, b.axis))
                return false;
            {
                float v = sf::Joystick::getAxisPosition(b.joystickId, b.axis);
                if (b.axisSign > 0)
                    return v > b.axisThreshold;
                else
                    return v < -b.axisThreshold;
            }
    }
    return false;
}

void InputManager::update(float dt) {
    if (!_hasFocus) {
        for (auto& [action, state] : _states) {
            state.pressed = false;
            state.justPressed = false;
            state.justReleased = false;
            state.holdTime = 0.f;
        }
        return;
    }

    for (auto& [action, bindings] : _bindings) {
        ActionState& st = _states[action];

        bool down = false;
        for (const auto& b : bindings) {
            if (isBindingActive(b)) {
                down = true;
                break;
            }
        }

        bool wasPressed = st.pressed;
        st.justPressed = (!wasPressed && down);
        st.justReleased = (wasPressed && !down);

        if (down) {
            st.holdTime += dt;
        } else {
            if (wasPressed) {
                st.lastReleaseHoldTime = st.holdTime;
            }
            st.holdTime = 0.f;
        }

        st.pressed = down;
    }
}

bool InputManager::isPressed(Action action) const {
    auto it = _states.find(action);
    return it != _states.end() && it->second.pressed;
}

bool InputManager::isJustPressed(Action action) const {
    auto it = _states.find(action);
    return it != _states.end() && it->second.justPressed;
}

bool InputManager::isJustReleased(Action action) const {
    auto it = _states.find(action);
    return it != _states.end() && it->second.justReleased;
}

bool InputManager::isShortPress(Action action, float threshold) const {
    auto it = _states.find(action);
    if (it == _states.end())
        return false;
    const auto& st = it->second;
    return st.justReleased && st.lastReleaseHoldTime > 0.f && st.lastReleaseHoldTime < threshold;
}

bool InputManager::isLongPress(Action action, float threshold) const {
    auto it = _states.find(action);
    if (it == _states.end())
        return false;
    const auto& st = it->second;
    return st.pressed && st.holdTime >= threshold;
}

const ActionState& InputManager::getState(Action action) const {
    return _states.at(action);
}
