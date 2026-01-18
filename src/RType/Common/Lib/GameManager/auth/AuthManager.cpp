#include "AuthManager.hpp"
#include <iostream>

void AuthManager::init(std::shared_ptr<Environment> env) {
    auto& ecs = env->getECS();
    cleanup(env);

    _usernameText.clear();
    _passwordText.clear();
    _currentAuthField = 0;
    _showAuthError = false;

    _authTitleEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _authTitleEntity, {"AUTHENTICATION", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 60,
                           sf::Color::Cyan, 650, 100});

    // Username
    _usernameLabelEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _usernameLabelEntity, {"Username:", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 30,
                               sf::Color::White, 600, 300});

    _usernameInputEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _usernameInputEntity, {"[ Click to type ]", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf",
                               30, sf::Color(150, 150, 150), 800, 300});
    ecs.registry.addComponent<InputTextComponent>(_usernameInputEntity, {"", "Username...", 12, false, false});

    // Password
    _passwordLabelEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _passwordLabelEntity, {"Password:", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 30,
                               sf::Color::White, 600, 400});

    _passwordInputEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _passwordInputEntity, {"[ Click to type ]", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf",
                               30, sf::Color(150, 150, 150), 800, 400});
    ecs.registry.addComponent<InputTextComponent>(_passwordInputEntity, {"", "Password...", 12, false, true});

    // Buttons
    _loginButtonEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _loginButtonEntity, {"[ LOGIN ]", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 40,
                             sf::Color::White, 650, 600});

    _registerButtonEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _registerButtonEntity, {"[ REGISTER ]", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 40,
                                sf::Color::White, 900, 600});

    _anonymousButtonEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _anonymousButtonEntity,
        {"[ ANONYMOUS LOGIN ]", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 30,
         sf::Color::Yellow, 750, 750});

    _authStatusEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _authStatusEntity,
        {"", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 24, sf::Color::Red, 700, 500});
}

void AuthManager::cleanup(std::shared_ptr<Environment> env) {
    auto& ecs = env->getECS();
    std::vector<Entity*> entities = {&_authTitleEntity,      &_usernameLabelEntity,   &_usernameInputEntity,
                                     &_passwordLabelEntity,  &_passwordInputEntity,   &_loginButtonEntity,
                                     &_registerButtonEntity, &_anonymousButtonEntity, &_authStatusEntity};
    for (auto* e : entities) {
        if (*e != static_cast<Entity>(-1)) {
            ecs.registry.destroyEntity(*e);
            *e = static_cast<Entity>(-1);
        }
    }
}

bool AuthManager::isMouseOverButton(sf::RenderWindow* window, float btnX, float btnY, float btnWidth, float btnHeight) {
    sf::Vector2i mousePos = window ? sf::Mouse::getPosition(*window) : sf::Mouse::getPosition();
    float mx = static_cast<float>(mousePos.x);
    float my = static_cast<float>(mousePos.y);
    return mx >= btnX && mx <= btnX + btnWidth && my >= btnY && my <= btnY + btnHeight;
}

void AuthManager::handleTextInput() {
    static int lastKey = -1;
    static bool backspaceWasPressed = false;
    static bool spaceWasPressed = false;

    int currentKey = -1;

    // Check letters A-Z
    for (int key = static_cast<int>(sf::Keyboard::Key::A); key <= static_cast<int>(sf::Keyboard::Key::Z); ++key) {
        if (sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(key))) {
            currentKey = key;
            break;
        }
    }
    // Check numbers 0-9
    if (currentKey == -1) {
        for (int key = static_cast<int>(sf::Keyboard::Key::Num0); key <= static_cast<int>(sf::Keyboard::Key::Num9);
             ++key) {
            if (sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(key))) {
                currentKey = key;
                break;
            }
        }
    }

    // Process new key press (only when different from last frame)
    if (currentKey != -1 && currentKey != lastKey && _inputText.length() < 20) {
        if (currentKey >= static_cast<int>(sf::Keyboard::Key::A) &&
            currentKey <= static_cast<int>(sf::Keyboard::Key::Z)) {
            char c = 'A' + (currentKey - static_cast<int>(sf::Keyboard::Key::A));
            if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) &&
                !sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift)) {
                c = c - 'A' + 'a';
            }
            _inputText += c;
        } else if (currentKey >= static_cast<int>(sf::Keyboard::Key::Num0) &&
                   currentKey <= static_cast<int>(sf::Keyboard::Key::Num9)) {
            char c = '0' + (currentKey - static_cast<int>(sf::Keyboard::Key::Num0));
            _inputText += c;
        }
    }
    lastKey = currentKey;

    // Space with debounce
    bool spacePressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);
    if (spacePressed && !spaceWasPressed && _inputText.length() < 20) {
        _inputText += ' ';
    }
    spaceWasPressed = spacePressed;

    // Backspace with debounce
    bool backspacePressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Backspace);
    if (backspacePressed && !backspaceWasPressed && !_inputText.empty()) {
        _inputText.pop_back();
    }
    backspaceWasPressed = backspacePressed;
}

void AuthManager::update(std::shared_ptr<Environment> env, sf::RenderWindow* window, bool hasFocus) {
    if (!hasFocus) {
        _mouseWasPressed = false;
        return;
    }

    bool mousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
    auto& ecs = env->getECS();

    // Button positions
    const float loginX = 650, loginY = 600, loginW = 200, loginH = 50;
    const float regX = 900, regY = 600, regW = 250, regH = 50;
    const float anonX = 750, anonY = 750, anonW = 400, anonH = 40;

    if (mousePressed && !_mouseWasPressed) {
        bool inputClicked = false;

        // Handle Input Fields
        auto& inputs = ecs.registry.getView<InputTextComponent>();
        auto& entities = ecs.registry.getEntities<InputTextComponent>();

        for (auto& entity : entities) {
            if (ecs.registry.hasComponent<TextComponent>(entity)) {
                auto& textComp = ecs.registry.getComponent<TextComponent>(entity);
                auto& input = ecs.registry.getComponent<InputTextComponent>(entity);

                if (isMouseOverButton(window, textComp.x, textComp.y, 400.0f, 40.0f)) {
                    input.isFocused = true;
                    _inputText = input.value;
                    inputClicked = true;
                    _currentAuthField =
                        (entity == _usernameInputEntity) ? 1 : ((entity == _passwordInputEntity) ? 2 : 0);
                } else {
                    input.isFocused = false;
                }
            }
        }

        if (!inputClicked) {
            _currentAuthField = 0;
            // Handle Buttons
            if (isMouseOverButton(window, loginX, loginY, loginW, loginH)) {
                if (!_usernameText.empty() && !_passwordText.empty() && env->hasFunction("sendLogin")) {
                    auto func =
                        env->getFunction<std::function<void(const std::string&, const std::string&)>>("sendLogin");
                    func(_usernameText, _passwordText);
                }
            } else if (isMouseOverButton(window, regX, regY, regW, regH)) {
                if (!_usernameText.empty() && !_passwordText.empty() && env->hasFunction("sendRegister")) {
                    auto func =
                        env->getFunction<std::function<void(const std::string&, const std::string&)>>("sendRegister");
                    func(_usernameText, _passwordText);
                }
            } else if (isMouseOverButton(window, anonX, anonY, anonW, anonH)) {
                if (env->hasFunction("sendAnonymousLogin")) {
                    auto func = env->getFunction<std::function<void()>>("sendAnonymousLogin");
                    func();
                }
            }
        }
    }
    _mouseWasPressed = mousePressed;

    // Handle Typing & Update Visuals
    auto& inputsMap = ecs.registry.getView<InputTextComponent>();
    auto& entitiesMap = ecs.registry.getEntities<InputTextComponent>();

    for (size_t i = 0; i < inputsMap.size(); ++i) {
        auto& input = inputsMap[i];
        Entity entity = entitiesMap[i];

        if (input.isFocused) {
            handleTextInput();
            input.updateText(_inputText);
        }

        if (_currentAuthField == 1) {
            _usernameText = input.value;
        } else if (_currentAuthField == 2) {
            _passwordText = input.value;
        }
        if (ecs.registry.hasComponent<TextComponent>(entity)) {
            auto& textComp = ecs.registry.getComponent<TextComponent>(entity);
            std::string displayStr = input.value;
            if (input.isHidden)
                displayStr = std::string(displayStr.length(), '*');

            if (input.value.empty() && !input.isFocused) {
                textComp.text = input.placeholder;
                textComp.color = sf::Color(150, 150, 150);
            } else {
                textComp.text = displayStr + (input.isFocused ? "|" : "");
                textComp.color = input.isFocused ? sf::Color::Yellow : sf::Color::White;
            }
        }
    }

    // Update Error Message
    if (ecs.registry.hasComponent<TextComponent>(_authStatusEntity)) {
        auto& text = ecs.registry.getComponent<TextComponent>(_authStatusEntity);
        if (_showAuthError) {
            text.text = "Login Failed! Try again.";
        } else {
            text.text = "";
        }
    }
}
