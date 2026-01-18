#include "ClientInputManager.hpp"
#include <cstdint>
#include "Components/NetworkComponents.hpp"
#include "Context.hpp"
#include "InputBinding.hpp"
#include "Network.hpp"

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

void ClientInputManager::update(engine::core::NetworkEngine& network, uint32_t tick, system_context& ctx) {
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
            st.holdTime += ctx.dt;
        } else {
            if (wasPressed) {
                st.lastReleaseHoldTime = st.holdTime;
            }
            st.holdTime = 0.f;
        }
        st.pressed = down;

        if (st.justPressed || st.justReleased || st.pressed) {
            createActionPacket(action, st, network, ctx);
        }
    }
}

void ClientInputManager::createActionPacket(Action name, ActionState state, engine::core::NetworkEngine& network,
                                            system_context& ctx) {
    ActionPacket packet;

    packet.action_name = name;
    packet.action_state = state;

    network.transmitEvent(network::GameEvents::C_INPUT, packet, ctx.tick);
    return;
}

InputSnapshot ClientInputManager::getCurrentInputSnapshot() const {
    InputSnapshot snapshot;
    for (const auto& [action, state] : _states) {
        if (state.pressed) {
            snapshot.actions[action] = true;
        }
    }
    return snapshot;
}