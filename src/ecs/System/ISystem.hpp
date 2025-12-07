/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ISystem.hpp
*/

#pragma once

#include <SFML/Graphics/Texture.hpp>
#include "../Registry/registry.hpp"
#include "ecs/ressource_manager/ressource_manager.hpp"

struct system_context {
    float dt;
    ResourceManager<sf::Texture>& texture_manager;
};

class ISystem {
    public:
        virtual ~ISystem() = default;
        virtual void init(Registry& registry) = 0;
        virtual void update(Registry& registry, system_context context) = 0;
};