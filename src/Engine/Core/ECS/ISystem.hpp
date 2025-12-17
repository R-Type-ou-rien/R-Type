 

#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

#include "Network/Client/Client.hpp"
#include "Network/Server/Server.hpp"
#include "InputManager.hpp"
#include "ressource_manager.hpp"

class Registry;

struct system_context {
    float dt;
    ResourceManager<sf::Texture>& texture_manager;
    std::optional<std::reference_wrapper<sf::RenderWindow>> window;
    std::optional<std::reference_wrapper<InputManager>> input;
    std::optional<std::reference_wrapper<Client>> network_client;
    std::optional<std::reference_wrapper<Server>> network_server;
    std::optional<std::reference_wrapper<std::vector<std::uint32_t>>> clients_id;
    std::optional<std::reference_wrapper<std::uint32_t>> client_id;
};

class ISystem {
   public:
    virtual ~ISystem() = default;

    virtual void update(Registry& registry, system_context context) = 0;
};
