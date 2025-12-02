/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ecs.hpp
*/

#pragma once

#include <SFML/Graphics.hpp>
#include "Registry/Registry.hpp"
#include "System/SystemManager/SystemManager.hpp"

class ECS {
    public:
        ECS() : systems(registry) {}

        ECS(unsigned int width = 800, unsigned int height = 600, const std::string& title = "R-Type");

        void update(float dt);

        void run();

        sf::RenderWindow& getWindow();

        Registry registry;
        SystemManager systems;

    private:
        sf::RenderWindow _window;
};