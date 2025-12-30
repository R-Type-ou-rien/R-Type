/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ISystem.hpp
*/

#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>

#include "Registry/registry.hpp"
#include "InputManager.hpp"
#include "ressource_manager.hpp"
#include "ResourceConfig.hpp"

struct system_context {
    float dt;
    ResourceManager<TextureAsset>& texture_manager;
    sf::RenderWindow& window;
    InputManager& input;
};

class ISystem {
   public:
    virtual ~ISystem() = default;

    virtual void update(Registry& registry, system_context context) = 0;
};
