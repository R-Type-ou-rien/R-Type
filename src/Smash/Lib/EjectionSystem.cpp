/*
** EPITECH PROJECT, 2025
** Smash
** File description:
** EjectionSystem.cpp
*/

#include "EjectionSystem.hpp"
#include "Components/StandardComponents.hpp"
#include "Components/GravityComponent.hpp"
#include <iostream>

void EjectionSystem::update(Registry& registry, system_context context) {
    auto& entities = registry.getEntities<EjectionComponent>();

    for (auto entity : entities) {
        auto& ejectionComp = registry.getComponent<EjectionComponent>(entity);
        
        if (ejectionComp.duration > 0) {
            ejectionComp.duration -= context.dt;
         
            if (ejectionComp.duration <= 0) {
                ejectionComp.duration = 0;
                if (registry.hasComponent<GravityComponent>(entity)) {
                    auto& gravity = registry.getComponent<GravityComponent>(entity);
                    gravity.vectorY = 0;
                }
            }
        }

        if (!ejectionComp.ejected)
            continue;
        if (!registry.hasComponent<Velocity2D>(entity))
            continue;
            
        auto& velocity = registry.getComponent<Velocity2D>(entity);
        velocity.vx += ejectionComp.ejectionForce.x;
        velocity.vy += ejectionComp.ejectionForce.y;
        
        if (registry.hasComponent<GravityComponent>(entity)) {
            auto& gravity = registry.getComponent<GravityComponent>(entity);
            gravity.vectorY = 0;
        }
        
        ejectionComp.ejected = false;
    }
}