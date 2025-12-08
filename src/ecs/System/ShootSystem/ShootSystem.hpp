/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ShootSystem.hpp
*/

#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Joystick.hpp>
#include "../ISystem.hpp"
#include "../../Components/Components.hpp"

class ShootSystem : public ISystem {
    public:
        void init(Registry& registry) override {}

        void update(Registry& registry, system_context context) override;
};
