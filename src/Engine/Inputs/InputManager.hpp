 

#pragma once

#include <optional>
#include <unordered_map>
#include <vector>

#include "ActionRegistry.hpp"
#include "Components/NetworkComponents.hpp"
#include "InputAction.hpp"
#include "InputBinding.hpp"
#include "InputState.hpp"
#include "Network/Client/Client.hpp"
#include "Network/Server/Server.hpp"

class InputManager {
   public:
    void bindAction(Action action, const InputBinding& binding);

    void update(float dt, std::optional<std::reference_wrapper<Client>> serv,
        std::optional<std::reference_wrapper<std::uint32_t>>);

    void setWindowHasFocus(bool focus) { _hasFocus = focus; }
    void setReceivedAction(InputPacket packet);

    bool isPressed(Action action) const;
    bool isJustPressed(Action action) const;
    bool isJustReleased(Action action) const;
    bool isShortPress(Action action, float threshold) const;
    bool isLongPress(Action action, float threshold) const;
    const ActionState& getState(Action action) const;

   private:
    ActionRegistry _actionRegistry;

    bool isBindingActive(const InputBinding& binding) const;

    std::unordered_map<Action, std::vector<InputBinding>> _bindings;
    std::unordered_map<Action, ActionState> _states;
    bool _hasFocus = true;
};

 
