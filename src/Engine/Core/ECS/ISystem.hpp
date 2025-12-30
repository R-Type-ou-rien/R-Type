/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ISystem.hpp
*/

#pragma once

#include "Registry/registry.hpp"
#include "ResourceConfig.hpp"

#ifdef CLIENT_BUILD
#include <SFML/Graphics/RenderWindow.hpp>
#include "InputManager.hpp"
#include "ressource_manager.hpp"

struct system_context {
    float dt;
    ResourceManager<TextureAsset>& texture_manager;
    sf::RenderWindow& window;
    InputManager& input;
};
#else // Pour le SERVER_BUILD
struct system_context {
    float dt;
};
#endif

class ISystem {
   public:
    virtual ~ISystem() = default;

    virtual void update(Registry& registry, system_context context) = 0;
};
