#pragma once

#include <memory>
#include <string>
#include <functional>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>
#include "ECS.hpp"
#include "GameEngineBase.hpp"
#include "../../../../../Engine/Lib/Components/StandardComponents.hpp"

class AuthManager {
   public:
    AuthManager() = default;
    ~AuthManager() = default;

    void init(std::shared_ptr<Environment> env);
    void update(std::shared_ptr<Environment> env, sf::RenderWindow* window, bool hasFocus);
    void cleanup(std::shared_ptr<Environment> env);

    void setAuthFailed() { _showAuthError = true; }
    void resetauthFailed() { _showAuthError = false; }

   private:
    void handleTextInput();
    bool isMouseOverButton(sf::RenderWindow* window, float btnX, float btnY, float btnWidth, float btnHeight);

    Entity _authTitleEntity = static_cast<Entity>(-1);
    Entity _usernameLabelEntity = static_cast<Entity>(-1);
    Entity _usernameInputEntity = static_cast<Entity>(-1);
    Entity _passwordLabelEntity = static_cast<Entity>(-1);
    Entity _passwordInputEntity = static_cast<Entity>(-1);
    Entity _loginButtonEntity = static_cast<Entity>(-1);
    Entity _registerButtonEntity = static_cast<Entity>(-1);
    Entity _anonymousButtonEntity = static_cast<Entity>(-1);
    Entity _authStatusEntity = static_cast<Entity>(-1);

    std::string _usernameText;
    std::string _passwordText;
    std::string _inputText;
    int _currentAuthField = 0;  // 0: None, 1: Username, 2: Password
    bool _showAuthError = false;
    bool _mouseWasPressed = false;
};
