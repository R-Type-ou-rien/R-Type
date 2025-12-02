/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ECS.cpp
*/

#include "ECS.hpp"

ECS::ECS(unsigned int width = 800, unsigned int height = 600, const std::string& title = "R-Type")
: _window(sf::VideoMode(width, height), title), 
systems(registry) 
{
    _window.setFramerateLimit(60);
}

void ECS::update(float dt) {
    systems.updateAll(dt);
}

void ECS::run() {
    sf::Clock clock;

    while (_window.isOpen()) {
        sf::Event event;
        while (_window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                _window.close();
        }
        sf::Time elapsed = clock.restart();
        float dt = elapsed.asSeconds();
        systems.updateAll(dt);
    }
}

sf::RenderWindow& ECS::getWindow() {
    return _window;
}