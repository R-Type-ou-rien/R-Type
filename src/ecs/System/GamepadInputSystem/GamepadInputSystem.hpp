/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** GamepadInputSystem.hpp
*/

#pragma once

#include <SFML/Window/Joystick.hpp>

#include "../../Components/Components.hpp"
#include "../ISystem.hpp"

class GamepadInputSystem : public ISystem {
   public:
    void init(Registry& registry) override {}

    void update(Registry& registry, system_context context) override;
};
