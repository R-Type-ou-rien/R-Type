#pragma once

#include <unordered_map>
#include <vector>
#include "InputAction.hpp"
#include "InputBinding.hpp"
#include "InputState.hpp"
#include "ActionRegistry.hpp"

template <class Derived>
class InputManagerBase {
   public:
    void bindAction(Action action, const InputBinding& binding) {
        _actionRegistry.registerAction(action);
        _bindings[action].push_back(binding);
    }

    bool isPressed(Action action) const {
        auto it = _states.find(action);
        return it != _states.end() && it->second.pressed;
    }

    bool isJustPressed(Action action) const {
        auto it = _states.find(action);
        return it != _states.end() && it->second.justPressed;
    }

    bool isJustReleased(Action action) const {
        auto it = _states.find(action);
        return it != _states.end() && it->second.justReleased;
    }

    bool isShortPress(Action action, float threshold) const {
        auto it = _states.find(action);
        if (it == _states.end())
            return false;
        const auto& st = it->second;
        return st.justReleased && st.lastReleaseHoldTime > 0.f && st.lastReleaseHoldTime < threshold;
    }

    bool isLongPress(Action action, float threshold) const {
        auto it = _states.find(action);
        if (it == _states.end())
            return false;
        const auto& st = it->second;
        return st.pressed && st.holdTime >= threshold;
    }

    const ActionState& getState(Action action) const { return _states.at(action); }

   protected:
    ActionRegistry _actionRegistry;
    std::unordered_map<Action, std::vector<InputBinding>> _bindings;
    std::unordered_map<Action, ActionState> _states;
};