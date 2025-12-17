 

#pragma once

#include <SFML/Window/Keyboard.hpp>

#include "Components/StandardComponents.hpp"
#include "ISystem.hpp"
#include "InputManager.hpp"

class InputSystem : public ISystem {
   public:
    explicit InputSystem(InputManager& input) : _input(input) {}

    void update(Registry& registry, system_context context) { _input.update(context.dt, context.network_client, context.client_id); }

   private:
    InputManager& _input;
};
