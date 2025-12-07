/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ShootSystem.hpp
*/

#pragma once

#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Keyboard.hpp>

#include "../../Components/Components.hpp"
#include "../ISystem.hpp"

class ShootSystem : public ISystem {
   public:
    void init(Registry& registry) override {}

    void update(Registry& registry, system_context context) override;
};
