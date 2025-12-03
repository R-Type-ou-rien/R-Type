/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ShootSystem.cpp
*/

#include "ShootSystem.hpp"

void ShootSystem::update(Registry& registry, float dt)
{
    auto& shooters = registry.getView<Shooter>();
    const auto& shooterEntities = registry.getEntities<Shooter>();

    for (std::size_t i = 0; i < shooterEntities.size(); ++i) {
        Entity entity = shooterEntities[i];
        auto& shooter = shooters[i];

        // Maj du cooldown
        shooter.timeSinceLastShot += dt;

        // -------- 1) On regarde si on veut tirer (clavier OU manette) --------
        bool shootPressed = false;

        // clavier
        if (sf::Keyboard::isKeyPressed(shooter.shootKey))
            shootPressed = true;

        // manette (si l'entité a un GamepadControl)
        if (registry.hasComponent<GamepadControl>(entity)) {
            auto& pad = registry.getComponent<GamepadControl>(entity);

            if (sf::Joystick::isConnected(pad.joystickId) &&
                sf::Joystick::isButtonPressed(pad.joystickId, pad.buttonShoot)) {
                shootPressed = true;
            }
        }

        // aucune entrée de tir -> on passe à l'entité suivante
        if (!shootPressed)
            continue;

        // -------- 2) Cooldown basé sur fireRate --------
        float delay = 1.0f / shooter.fireRate;
        if (shooter.timeSinceLastShot < delay)
            continue;

        shooter.timeSinceLastShot = 0.f;

        // -------- 3) On a le droit de tirer → on crée un projectile --------
        if (!registry.hasComponent<Position2D>(entity))
            continue;

        auto& shooterPos = registry.getComponent<Position2D>(entity);

        // Création du projectile
        Entity proj = registry.createEntity();

        Position2D projPos{
            shooterPos.x + 50.f,   // petit offset devant le joueur
            shooterPos.y
        };

        Velocity2D projVel{
            shooter.projectileSpeed,
            0.f
        };

        Projectile projComp{
            shooter.projectileLifetime
        };

        // À adapter en fonction de ta spritesheet
        Sprite2D projSprite{
            "content/sprites/r-typesheet42.gif",
            0,   // rectLeft
            16,  // rectTop (par ex. ligne du projectile)
            32,  // rectWidth
            16,  // rectHeight
            3.0f // scale
        };

        registry.addComponent(proj, projPos);
        registry.addComponent(proj, projVel);
        registry.addComponent(proj, projComp);
        registry.addComponent(proj, projSprite);
    }
}
