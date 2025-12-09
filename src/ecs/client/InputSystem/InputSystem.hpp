/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** InputSystem.hpp
*/

#pragma once

#include <SFML/Window/Keyboard.hpp>

#include "../../Input/InputManager.hpp"
#include "../../common/Components/Components.hpp"
#include "../../common/ISystem.hpp"

class InputSystem : public ISystem {
   public:
    explicit InputSystem(InputManager& input) : _input(input) {}

    void update(Registry& registry, system_context context) { _input.update(context.dt); }

   private:
    InputManager& _input;
};
