/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ECS.hpp
*/

#pragma once

#include <optional>
#include <string>

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Texture.hpp>

#include "ISystem.hpp"
#include "InputSystem.hpp"
#include "Registry/registry.hpp"
#include "SystemManager/SystemManager.hpp"

class ECS {
   public:
    ECS() : systems(registry) {}

    void update(system_context context) { systems.updateAll(context); }

   public:
    Registry registry;
    SystemManager systems;
    ResourceManager<sf::Texture> _textureManager;
    // InputManager input;
};
