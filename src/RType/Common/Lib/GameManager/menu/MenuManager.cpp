#include "MenuManager.hpp"
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <cmath>
#include <ctime>
#include "../../../../../Engine/Lib/Components/StandardComponents.hpp"

void MenuManager::init(std::shared_ptr<Environment> env) {
    auto& ecs = env->getECS();
    cleanup(env);

    _menuTitleEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _menuTitleEntity,
        {"R-TYPE", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 100, sf::Color::Cyan, 750, 200});

    _playButtonEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _playButtonEntity, {"[ PLAY ]", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 60,
                            sf::Color::White, 820, 500});
}

void MenuManager::cleanup(std::shared_ptr<Environment> env) {
    auto& ecs = env->getECS();
    if (_menuTitleEntity != static_cast<Entity>(-1)) {
        ecs.registry.destroyEntity(_menuTitleEntity);
        _menuTitleEntity = static_cast<Entity>(-1);
    }
    if (_playButtonEntity != static_cast<Entity>(-1)) {
        ecs.registry.destroyEntity(_playButtonEntity);
        _playButtonEntity = static_cast<Entity>(-1);
    }
}

bool MenuManager::isMouseOverButton(sf::RenderWindow* window, float btnX, float btnY, float btnWidth, float btnHeight) {
    sf::Vector2i mousePos = window ? sf::Mouse::getPosition(*window) : sf::Mouse::getPosition();
    float mx = static_cast<float>(mousePos.x);
    float my = static_cast<float>(mousePos.y);
    return mx >= btnX && mx <= btnX + btnWidth && my >= btnY && my <= btnY + btnHeight;
}

void MenuManager::update(std::shared_ptr<Environment> env, sf::RenderWindow* window, bool hasFocus) {
    if (!hasFocus) {
        _mouseWasPressed = false;
        _enterWasPressed = false;
        return;
    }

    bool mousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
    bool enterPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter);
    const float playBtnX = 820.0f, playBtnY = 500.0f, playBtnW = 200.0f, playBtnH = 70.0f;

    if (mousePressed && !_mouseWasPressed && isMouseOverButton(window, playBtnX, playBtnY, playBtnW, playBtnH)) {
        env->setGameState(Environment::GameState::LOBBY_LIST);
    }
    _mouseWasPressed = mousePressed;

    if (enterPressed && !_enterWasPressed) {
        env->setGameState(Environment::GameState::LOBBY_LIST);
    }
    _enterWasPressed = enterPressed;

    auto& ecs = env->getECS();
    if (_playButtonEntity != static_cast<Entity>(-1) && ecs.registry.hasComponent<TextComponent>(_playButtonEntity)) {
        auto& text = ecs.registry.getComponent<TextComponent>(_playButtonEntity);
        float pulse = (std::sin(static_cast<float>(std::clock()) / 500.0f) + 1.0f) / 2.0f;
        text.color =
            isMouseOverButton(window, playBtnX, playBtnY, playBtnW, playBtnH)
                ? sf::Color::Yellow
                : sf::Color(static_cast<uint8_t>(155 + 100 * pulse), static_cast<uint8_t>(155 + 100 * pulse), 255);
    }
}
