/*
** EPITECH PROJECT, 2025
** Smash
** File description:
** GameManager.cpp
*/

#include "GameManager.hpp"

#include "Components/GroundComponent.hpp"
#include "Components/TransformComponent.hpp"
#include "Components/Sprite/Sprite2D.hpp"
#include "Components/TextComponent.hpp"
#include "../Lib/PercentageHealth.hpp"

#include "Systems/GravitySystem/GravitySystem.hpp"
#include "Systems/AnimationSystem/AnimationSystem.hpp"

GameManager::GameManager() {}

void GameManager::init(ECS& ecs) {
    ecs.systems.addSystem<AnimationSystem>();
    ecs.systems.addSystem<GravitySystem>();

    this->setBackground(ecs);
    this->setPlatform(ecs);

    _player1 = std::make_unique<Player>(ecs, std::pair<float, float>{300.f, 300.f}, 1);
    _player1->setSpriteAnimation("src2/Smash/Client/Content/player1-Sheet.png");
    ecs.input.bindAction("move_left", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Left});
    ecs.input.bindAction("move_right", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Right});
    ecs.input.bindAction("jump", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Up});
    ecs.input.bindAction("attack_simple", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::A});
    ecs.input.bindAction("attack_heavy", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Z});
    ecs.input.bindAction("special_attack", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::E});
    ecs.input.bindAction("block", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::R});

    _player2 = std::make_unique<Player>(ecs, std::pair<float, float>{700.f, 300.f}, 2);
    _player2->setSpriteAnimation("src2/Smash/Client/Content/player2-Sheet.png");
    _player2->flipXSprite(true);
    ecs.input.bindAction("move_left2", InputBinding{InputDeviceType::GamepadAxis, sf::Keyboard::Key::Left, sf::Mouse::Button::Left, 0, 0, sf::Joystick::Axis::X, 50.f, -1});
    ecs.input.bindAction("move_right2", InputBinding{InputDeviceType::GamepadAxis, sf::Keyboard::Key::Right, sf::Mouse::Button::Left, 0, 0, sf::Joystick::Axis::X, 50.f, 1});
    ecs.input.bindAction("jump2", InputBinding{InputDeviceType::GamepadAxis, sf::Keyboard::Key::Unknown, sf::Mouse::Button::Left, 0, 0, sf::Joystick::Axis::Y, 50.f, -1});
    ecs.input.bindAction("move_left2", InputBinding{InputDeviceType::GamepadAxis, sf::Keyboard::Key::Left, sf::Mouse::Button::Left, 0, 0, sf::Joystick::Axis::PovX, 50.f, -1});
    ecs.input.bindAction("move_right2", InputBinding{InputDeviceType::GamepadAxis, sf::Keyboard::Key::Right, sf::Mouse::Button::Left, 0, 0, sf::Joystick::Axis::PovX, 50.f, 1});
    ecs.input.bindAction("jump2", InputBinding{InputDeviceType::GamepadAxis, sf::Keyboard::Key::Unknown, sf::Mouse::Button::Left, 0, 0, sf::Joystick::Axis::PovY, 50.f, -1});
    ecs.input.bindAction("attack_simple2", InputBinding{InputDeviceType::GamepadButton, sf::Keyboard::Key::Unknown, sf::Mouse::Button::Left, 0, 0});
    ecs.input.bindAction("attack_heavy2", InputBinding{InputDeviceType::GamepadButton, sf::Keyboard::Key::Unknown, sf::Mouse::Button::Left, 0, 1});
    ecs.input.bindAction("special_attack2", InputBinding{InputDeviceType::GamepadButton, sf::Keyboard::Key::Unknown, sf::Mouse::Button::Left, 0, 2});
    ecs.input.bindAction("block2", InputBinding{InputDeviceType::GamepadButton, sf::Keyboard::Key::Unknown, sf::Mouse::Button::Left, 0, 3});

    _UiPercentPlayer1 = ecs.registry.createEntity();
    _UiPercentPlayer2 = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(_UiPercentPlayer1, TextComponent{"0%", "src2/Smash/Client/Content/font.otf", 40, sf::Color::Blue, 200, 700});
    ecs.registry.addComponent<TextComponent>(_UiPercentPlayer2, TextComponent{"0%", "src2/Smash/Client/Content/font.otf", 40, sf::Color::Red, 800, 700});

    _UiLivesPlayer1 = ecs.registry.createEntity();
    _UiLivesPlayer2 = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(_UiLivesPlayer1, TextComponent{"3", "src2/Smash/Client/Content/font.otf", 20, sf::Color::Blue, 200, 750});
    ecs.registry.addComponent<TextComponent>(_UiLivesPlayer2, TextComponent{"3", "src2/Smash/Client/Content/font.otf", 20, sf::Color::Red, 800, 750});
    return;
}

void GameManager::update(ECS& ecs) {
    if (this->checkEnd(ecs)) {
        _player1->setPlayable(false);
        _player2->setPlayable(false);
        _player1->reset();
        _player2->reset();
        _player1->setPosition({300.f, 300.f});
        _player2->setPosition({700.f, 300.f});
        return;
    }
    _player1->checkMove();
    _player2->checkMove();
    _player1->updateAnimation();
    _player2->updateAnimation();

    this->updateUIHealthPercentage(ecs);
    return;
}   

void GameManager::loadInputSetting(ECS& ecs) {

    return;
}

// Private Methods

void GameManager::setBackground(ECS& ecs) {
    Entity background = ecs.registry.createEntity();

    Sprite2D sprite;
    std::string pathname = "src2/Smash/Client/Content/bg_smash.jpg";
    sprite.rect = {0, 0, 1920, 800};
    sprite.layer = RenderLayer::Background;
    if (ecs._textureManager.is_loaded(pathname)) {
        sprite.handle = ecs._textureManager.get_handle(pathname).value();
    } else {
        sprite.handle = ecs._textureManager.load_resource(pathname, sf::Texture(pathname));
    }

    ecs.registry.addComponent<transform_component_s>(background, {500.f, 400.f});
    ecs.registry.addComponent<Sprite2D>(background, sprite);
    return;
}

void GameManager::setPlatform(ECS& ecs) {
    Entity platform = ecs.registry.createEntity();

    Sprite2D sprite;
    std::string pathname = "src2/Smash/Client/Content/platform.png";
    sprite.rect = {0, 0, 500, 100};
    sprite.layer = RenderLayer::Midground;
    if (ecs._textureManager.is_loaded(pathname)) {
        sprite.handle = ecs._textureManager.get_handle(pathname).value();
    } else {
        sprite.handle = ecs._textureManager.load_resource(pathname, sf::Texture(pathname));
    }

    ecs.registry.addComponent<transform_component_s>(platform, {500.f, 600.f});
    ecs.registry.addComponent<GroundComponent>(platform, {{250, 550, 500, 100}, true});
    ecs.registry.addComponent<Sprite2D>(platform, sprite);
    return;
}

void GameManager::updateUIHealthPercentage(ECS& ecs) {
    if (_player1 && _player2) {
        auto health1 = _player1->getCurrentHealthPercentage();
        auto health2 = _player2->getCurrentHealthPercentage();
        ecs.registry.getComponent<TextComponent>(_UiPercentPlayer1).text = std::to_string(static_cast<int>(health1)) + "%";
        ecs.registry.getComponent<TextComponent>(_UiPercentPlayer2).text = std::to_string(static_cast<int>(health2)) + "%";
        ecs.registry.getComponent<TextComponent>(_UiLivesPlayer1).text = "lives: " + std::to_string(ecs.registry.getComponent<PercentageHealth>(_player1->getId()).totalLifeRespawn);
        ecs.registry.getComponent<TextComponent>(_UiLivesPlayer2).text = "lives: " + std::to_string(ecs.registry.getComponent<PercentageHealth>(_player2->getId()).totalLifeRespawn);
    }
}

bool GameManager::checkEnd(ECS& ecs) {
    if (_gameOver)
        return true;
    if (_player1->OutOfMap(-100.f, -100.f, 1100.f, 900.f)) {
        PercentageHealth &player1Health = ecs.registry.getComponent<PercentageHealth>(_player1->getId());
        player1Health.totalLifeRespawn -= 1;
        if (player1Health.totalLifeRespawn > 0) {
            _player1->reset();
            return false;
        }
        _gameOver = true;
        TextComponent& textWin = ecs.registry.getComponent<TextComponent>(_UiPercentPlayer2);
        textWin.text = "Player 2 Wins!";
        textWin.characterSize = 75;
        textWin.x = 200;
        textWin.y = 300;
        ecs.registry.getComponent<TextComponent>(_UiPercentPlayer1).text = "Player 1 LOSES!";
        ecs.registry.getComponent<TextComponent>(_UiLivesPlayer1).text = "lives: " + std::to_string(ecs.registry.getComponent<PercentageHealth>(_player1->getId()).totalLifeRespawn);
        ecs.registry.getComponent<TextComponent>(_UiLivesPlayer2).text = "lives: " + std::to_string(ecs.registry.getComponent<PercentageHealth>(_player2->getId()).totalLifeRespawn);
        return true;
    }
    if (_player2->OutOfMap(-100.f, -100.f, 1100.f, 900.f)) {
        PercentageHealth &player2Health = ecs.registry.getComponent<PercentageHealth>(_player2->getId());
        player2Health.totalLifeRespawn -= 1;
        if (player2Health.totalLifeRespawn > 0) {
            _player2->reset();
            return false;
        }
        _gameOver = true;
        TextComponent& textWin = ecs.registry.getComponent<TextComponent>(_UiPercentPlayer1);
        textWin.text = "Player 1 Wins!";
        textWin.characterSize = 75;
        textWin.x = 200;
        textWin.y = 300;
        ecs.registry.getComponent<TextComponent>(_UiPercentPlayer2).text = "Player 2 LOSES!";
        ecs.registry.getComponent<TextComponent>(_UiPercentPlayer2).x = 525;
        ecs.registry.getComponent<TextComponent>(_UiLivesPlayer1).text = "lives: " + std::to_string(ecs.registry.getComponent<PercentageHealth>(_player1->getId()).totalLifeRespawn);
        ecs.registry.getComponent<TextComponent>(_UiLivesPlayer2).text = "lives: " + std::to_string(ecs.registry.getComponent<PercentageHealth>(_player2->getId()).totalLifeRespawn);
        return true;
    }
    return false;
}
