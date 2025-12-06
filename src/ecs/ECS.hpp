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
#include "System/ISystem.hpp"
#include "System/SystemManager/SystemManager.hpp"
#include "../utils/slot_map/slot_map.hpp"

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

        void run() {
            sf::Clock clock;
            SlotMap<sf::Texture> texture_manager;
            system_context context = {0, texture_manager};

            while (_window.isOpen()) {
                while (const std::optional event = _window.pollEvent()) {
                    if (event->is<sf::Event::Closed>())
                        _window.close();
                }
                sf::Time elapsed = clock.restart();
                context.dt = elapsed.asSeconds();
                systems.updateAll(context);
            }
        }

        // sf::RenderWindow& getWindow() {
        //     return _window;
        // }

    public:
        Registry registry;
        SystemManager systems;
        SlotMap<sf::Texture> _textureManager;

    private:
        sf::RenderWindow _window;
};