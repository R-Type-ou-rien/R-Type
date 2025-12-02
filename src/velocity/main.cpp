/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** test movement
*/
#include <iostream>
#include "Velocity.hpp"

int main()
{
    Ecs::VelocitySystem velocitySystem;
    Ecs::Placement_t placement{0.0, 0.0, 0.0f};
    Ecs::Velocity_t velocity{5.0f, 3.0f};
    float deltaTime = 0.016f; // ~60 FPS
    // float dt = elapsed.asSeconds();

    std::cout << "x=" << placement.pos_x 
              << ", y=" << placement.pos_y << std::endl;
    velocitySystem.UpdatePosition(placement, velocity, deltaTime);
    std::cout << "x=" << placement.pos_x 
              << ", y=" << placement.pos_y << std::endl;
}
