/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** InputSystem.hpp
*/

#pragma once

#include "../../Input/InputManager.hpp"
#include "../ISystem.hpp"

class InputSystem : public ISystem {
   public:
    InputSystem(InputManager& input) : _input(input) {}

    void update(Registry& registry, float dt) { _input.update(dt); }

   private:
    InputManager& _input;
};
