/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ECS.hpp
*/

#pragma once

#include <optional>
#include <string>
#include <functional>

// #include <SFML/Graphics.hpp>
// #include <SFML/Graphics/Texture.hpp>

#include "ISystem.hpp"
#include "../../Inputs/InputSystem.hpp"
#include "../../Inputs/InputManager/InputManager.hpp"
#include "../../Resources/ResourceConfig.hpp"
#include "Registry/registry.hpp"
#include "SystemManager/SystemManager.hpp"

class ECS {
   public:
    ECS(ResourceManager<TextureAsset>& textureMgr, InputManager& inputMgr)
        : systems(registry), input(inputMgr), _textureManager(textureMgr) {}

    void update(system_context context) { systems.updateAll(context); }

   public:
    Registry registry;
    SystemManager systems;
    InputManager& input;
    ResourceManager<TextureAsset>& _textureManager;
};
