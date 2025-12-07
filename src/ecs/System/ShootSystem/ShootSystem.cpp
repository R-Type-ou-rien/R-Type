/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ShootSystem.cpp
*/

#include "ShootSystem.hpp"

#include "ecs/Components/Components.hpp"

void ShootSystem::update(Registry& registry, system_context context) {
    auto& shooters = registry.getView<Shooter>();
    const auto& shooterEntities = registry.getEntities<Shooter>();

    for (std::size_t i = 0; i < shooterEntities.size(); ++i) {
        Entity entity = shooterEntities[i];
        auto& shooter = shooters[i];

        // Maj du cooldown
        shooter.timeSinceLastShot += context.dt;

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
        if (!registry.hasComponent<sprite2D_component_s>(entity))
            continue;

        auto& shooterPos = registry.getComponent<transform_component_s>(entity);

        // Création du projectile
        Entity proj = registry.createEntity();

        transform_component_s projPos{shooterPos.x + 50.f,  // petit offset devant le joueur
                                      shooterPos.y,
                                      {3.0f, 3.0f}};

        Velocity2D projVel{shooter.projectileSpeed, 0.f};

        Projectile projComp{shooter.projectileLifetime};

        // À adapter en fonction de ta spritesheet
        handle_t<sf::Texture> handle = context.texture_manager.load_resource(
            "content/sprites/r-typesheet42.gif", sf::Texture("content/sprites/r-typesheet42.gif"));
        sprite2D_component_s projSprite;
        projSprite.handle = handle;
        projSprite.animation_speed = 0.5f;
        projSprite.current_animation_frame = 0;
        projSprite.dimension.position = {0, 16};
        projSprite.dimension.size = {32, 16};
        projSprite.z_index = 1;

        registry.addComponent<transform_component_s>(proj, projPos);
        registry.addComponent<Velocity2D>(proj, projVel);
        registry.addComponent<Projectile>(proj, projComp);
        registry.addComponent<sprite2D_component_s>(proj, projSprite);
    }
}
