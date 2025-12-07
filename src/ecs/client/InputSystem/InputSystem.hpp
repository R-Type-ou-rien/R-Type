/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** InputSystem.hpp
*/

#pragma once

#include <SFML/Window/Keyboard.hpp>
#include "../../common/ISystem.hpp"
#include "../../Components/Components.hpp"

class InputSystem : public ISystem {
    public:
        void init(Registry& registry) override {
        }

        void update(Registry& registry, system_context context) override;
};