#pragma once

#include <memory>
#include <functional>
#include <SFML/Graphics/RenderWindow.hpp>
#include "ECS.hpp"
#include "GameEngineBase.hpp"

class MenuManager {
   public:
    MenuManager() = default;
    ~MenuManager() = default;

    void init(std::shared_ptr<Environment> env);
    void update(std::shared_ptr<Environment> env, sf::RenderWindow* window, bool hasFocus);
    void cleanup(std::shared_ptr<Environment> env);

   private:
    bool isMouseOverButton(sf::RenderWindow* window, float btnX, float btnY, float btnWidth, float btnHeight);

    Entity _menuTitleEntity = static_cast<Entity>(-1);
    Entity _playButtonEntity = static_cast<Entity>(-1);
    Entity _menuBackgroundEntity = static_cast<Entity>(-1);

    bool _mouseWasPressed = false;
    bool _enterWasPressed = false;
};
