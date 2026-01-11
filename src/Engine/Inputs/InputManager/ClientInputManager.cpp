#include "ClientInputManager.hpp"
#include "InputBinding.hpp"

bool ClientInputManager::isBindingActive(const InputBinding& b) const {
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

void ClientInputManager::update(float dt) {
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
        ActionState& st = this->_states[action];

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
