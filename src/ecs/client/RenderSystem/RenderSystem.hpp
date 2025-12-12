/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** RenderSystem.hpp
*/

#pragma once

#include <unordered_map>
#include <string>

#include <SFML/Graphics.hpp>

#include "ecs/common/Components/Components.hpp"
#include "ecs/common/ISystem.hpp"

class RenderSystem : public ISystem {
   public:
    RenderSystem() = default;

    void update(Registry& registry, system_context context) override;

   private:
    sf::Texture& getTexture(const std::string& path);

    void drawEntity(const transform_component_s& transform, const sprite2D_component_s& spriteData,
                    const system_context& context);
};
