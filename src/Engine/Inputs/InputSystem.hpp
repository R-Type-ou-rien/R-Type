/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** InputSystem.hpp
*/

#pragma once

// #include <SFML/Window/Keyboard.hpp>

#include "Components/StandardComponents.hpp"
#include "ISystem.hpp"
#include "InputConfig.hpp"

class InputSystem : public ISystem {
   public:
    explicit InputSystem(InputManager& input) : _input(input) {}

    void update(Registry& registry, system_context context) { _input.update(context.dt); }

   private:
    InputManager& _input;
};
