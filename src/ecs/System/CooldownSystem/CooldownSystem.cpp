/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** CooldownSystem.cpp
*/

#include "CooldownSystem.hpp"

#include <algorithm>  // std::clamp (C++17)

#include "ecs/Components/Components.hpp"

void CooldownSystem::update(Registry& registry, system_context) {
    auto& shooters = registry.getView<Shooter>();
    const auto& entities = registry.getEntities<Shooter>();

    // paramètres visuels de la barre
    const float barWidth = 40.f;
    const float barHeight = 6.f;
    const float yOffset = 30.f;  // hauteur au-dessus du centre du joueur

    for (std::size_t i = 0; i < entities.size(); ++i) {
        Entity entity = entities[i];
        auto& shooter = shooters[i];

        if (!registry.hasComponent<transform_component_s>(entity))
            continue;

        auto& pos = registry.getComponent<transform_component_s>(entity);

        // temps nécessaire entre deux tirs
        const float delay = 1.0f / shooter.fireRate;

        // ratio entre 0 (vient de tirer) et 1 (prêt à tirer)
        float ratio = shooter.timeSinceLastShot / delay;
        ratio = std::clamp(ratio, 0.0f, 1.0f);

        // barre de fond (gris)
        sf::RectangleShape backgroundBar;
        backgroundBar.setSize({barWidth, barHeight});
        backgroundBar.setFillColor(sf::Color(80, 80, 80, 180));
        backgroundBar.setOrigin({barWidth / 2.f, barHeight / 2.f});
        backgroundBar.setPosition({pos.x, pos.y - yOffset});

        // barre de cooldown (vert qui se remplit)
        sf::RectangleShape cooldownBar;
        cooldownBar.setSize({barWidth * ratio, barHeight});
        cooldownBar.setFillColor(sf::Color(100, 255, 100, 220));
        cooldownBar.setOrigin({barWidth / 2.f, barHeight / 2.f});
        // pour que ça se remplisse de gauche à droite, on déplace un peu
        cooldownBar.setPosition({pos.x - (barWidth * (1.f - ratio)) / 2.f, pos.y - yOffset});

        _window.draw(backgroundBar);
        _window.draw(cooldownBar);
    }
}
