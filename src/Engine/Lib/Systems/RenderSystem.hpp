/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** RenderSystem.hpp
*/

#pragma once

#include <unordered_map>
#include <string>

<<<<<<< HEAD:src/Engine/Lib/Systems/RenderSystem.hpp
#include "Components/StandardComponents.hpp"
#include "ISystem.hpp"
=======
#include <SFML/Graphics.hpp>

#include "ecs/common/Components/Components.hpp"
#include "ecs/common/ISystem.hpp"
>>>>>>> 2e0d1a29fa2d0e6b3713286aabdb39628515dfd4:src/ecs/client/RenderSystem/RenderSystem.hpp

class RenderSystem : public ISystem {
   public:
    RenderSystem() = default;

    void update(Registry& registry, system_context context) override;

   private:
    sf::Texture& getTexture(const std::string& path);

    void drawEntity(const transform_component_s& transform, const sprite2D_component_s& spriteData,
                    const system_context& context);
};
