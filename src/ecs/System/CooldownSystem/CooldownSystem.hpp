/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** CooldownSystem.hpp
*/

#pragma once

#include <SFML/Graphics.hpp>
#include "../ISystem.hpp"
#include "../../Components/Components.hpp"

class CooldownSystem : public ISystem {
    public:
        explicit CooldownSystem(sf::RenderWindow &window)
            : _window(window) {}

        void init(Registry &registry) override {}

        void update(Registry &registry, float dt) override;

    private:
        sf::RenderWindow &_window;
};
