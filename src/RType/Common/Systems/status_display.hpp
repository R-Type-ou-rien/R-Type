/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Status Display System for R-Type style UI rendering
*/

#pragma once

#include <SFML/Graphics.hpp>
#include "ISystem.hpp"
#include "registry.hpp"
#include "Context.hpp"
#include "../Components/status_display_components.hpp"

class StatusDisplaySystem : public ISystem {
   public:
    void update(Registry& registry, system_context context) override;

   private:
    void drawChargeBar(Registry& registry, system_context& context);
    void drawLives(Registry& registry, system_context& context);
    void drawScore(Registry& registry, system_context& context);

    sf::Font _font;
    bool _fontLoaded = false;
};
