/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ISystem.hpp
*/

#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <functional>
#include <optional>

#include "Network/Client/Client.hpp"
#include "Network/Server/Server.hpp"
#include "Registry/registry.hpp"
#include "InputManager.hpp"
#include "ressource_manager.hpp"

struct system_context {
    float dt;
    std::optional<std::reference_wrapper<ResourceManager<sf::Texture>>> texture_manager;
    std::optional<std::reference_wrapper<sf::RenderWindow>> window;
    std::optional<std::reference_wrapper<InputManager>> input;
    std::optional<std::reference_wrapper<Client>> network_client;
    std::optional<std::reference_wrapper<Server>> network_server;
};

class ISystem {
   public:
    virtual ~ISystem() = default;

    virtual void update(Registry& registry, system_context context) = 0;
};
