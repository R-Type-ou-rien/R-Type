/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ECS.hpp
*/

#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <optional>
#include "Registry/registry.hpp"
#include "ISystem.hpp"
#include "SystemManager/SystemManager.hpp"

class ECS {
    public:
        ECS() : systems(registry) {};

        ECS(unsigned int width, unsigned int height, const std::string& title = "R-Type")
        : _window(sf::VideoMode({width, height}), title), 
        systems(registry) 
        {
            _window.setFramerateLimit(60);
        }

        void update(system_context context) {
            systems.updateAll(context);
        }

        sf::RenderWindow& getWindow() {
            return _window;
        }

    public:
        Registry registry;
        SystemManager systems;
        ResourceManager<sf::Texture> _textureManager;

    private:
        sf::RenderWindow _window;
};