/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** GamepadInputSystem.hpp
*/

#pragma once

#include <SFML/Window/Joystick.hpp>
#include "../ISystem.hpp"
#include "../../Components/Components.hpp"

class GamepadInputSystem : public ISystem {
    public:
        void init(Registry& registry) override {
        }

        void update(Registry& registry, float dt) override;
};
