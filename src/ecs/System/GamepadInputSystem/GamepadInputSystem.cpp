/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** GamepadInputSystem.cpp
*/

#include "GamepadInputSystem.hpp"
#include <cmath> // std::abs

void GamepadInputSystem::update(Registry& registry, float dt) {
    const auto& entities = registry.getEntities<GamepadControl>();
    auto& controls = registry.getView<GamepadControl>();

    for (std::size_t i = 0; i < entities.size(); ++i) {
        Entity entity = entities[i];
        auto& ctrl = controls[i];

        if (!registry.hasComponent<Velocity2D>(entity))
            continue;

        auto& vel = registry.getComponent<Velocity2D>(entity);

        // Si la manette n'est pas connectée : on arrête le mouvement
        if (!sf::Joystick::isConnected(ctrl.joystickId)) {
            vel.vx = 0.f;
            vel.vy = 0.f;
            continue;
        }

        // On récupère la position des axes (-100 à 100)
        float x = sf::Joystick::getAxisPosition(ctrl.joystickId, ctrl.axisX);
        float y = sf::Joystick::getAxisPosition(ctrl.joystickId, ctrl.axisY);

        // Dead zone : évite que le perso bouge tout seul
        if (std::abs(x) < ctrl.deadZone) x = 0.f;
        if (std::abs(y) < ctrl.deadZone) y = 0.f;

        // Normalisation en [-1, 1]
        x /= 100.f;
        y /= 100.f;

        // On applique la vitesse
        vel.vx = x * ctrl.speed;
        vel.vy = y * ctrl.speed;
        // (y négatif = vers le haut, ce qui colle avec ton MoveSystem + SFML)
    }
}
