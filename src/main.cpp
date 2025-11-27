/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** main.cpp
*/

#include "ecs/ECS.hpp"
#include "ecs/Components/Components.hpp"
#include "ecs/System/MoveSystem/MoveSystem.hpp"
#include "ecs/System/RenderSystem/RenderSystem.hpp"
#include "ecs/System/InputSystem/InputSystem.hpp"

int main() {
    ECS ecs(800, 600, "R-Type - ECS Engine");

    ecs.systems.addSystem<InputSystem>();
    ecs.systems.addSystem<MoveSystem>();
    ecs.systems.addSystem<RenderSystem>(ecs.getWindow());

    EntityID player = ecs.registry.createEntity();
    
    ecs.registry.addComponent(player, Position2D{100.0f, 300.0f});
    ecs.registry.addComponent(player, Velocity2D{0.0f, 0.0f});
    ecs.registry.addComponent(player, InputControl{
        sf::Keyboard::Up, sf::Keyboard::Down,
        sf::Keyboard::Left, sf::Keyboard::Right,
        300.0f 
    });
    ecs.registry.addComponent(player, Sprite2D{"content/sprites/r-typesheet42.gif", 0, 0, 32, 16, 3.0f});

    auto& window = ecs.getWindow();

    sf::Clock clock;
    while (window.isOpen()) {
        sf::Event event;

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        sf::Time elapsed = clock.restart();
        float dt = elapsed.asSeconds();
        ecs.update(dt);
    }

    // ecs.run();
    return 0;
}