/*
** EPITECH PROJECT, 2025
** Smash
** File description:
** GameManager.cpp
*/

#include "GameManager.hpp"

#include "Components/GroundComponent.hpp"
#include <memory>
#include <utility>
#include <vector>
#include <string>
#include "Components/StandardComponents.hpp"
#include "Components/Sprite/Sprite2D.hpp"
#include "../Lib/PercentageHealth.hpp"
#include "../../../Engine/Inputs/InputBinding.hpp"

#include "Systems/GravitySystem/GravitySystem.hpp"
#include "Systems/AnimationSystem/AnimationSystem.hpp"
#include "../Lib/EjectionSystem.hpp"

GameManager::GameManager() {}

void GameManager::init(ECS& ecs) {
    ecs.systems.addSystem<AnimationSystem>();
    ecs.systems.addSystem<GravitySystem>();
    ecs.systems.addSystem<EjectionSystem>();

    this->setBackground(ecs);
    this->setPlatform(ecs);

    _player1 = std::make_unique<Player>(ecs, std::pair<float, float>{300.f, 300.f}, 1);
    _player1->setSpriteAnimation("src/Smash/Content/player1-Sheet.png");
    ecs.input.bindAction("move_left", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Left});
    ecs.input.bindAction("move_right", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Right});
    ecs.input.bindAction("jump", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Up});
    ecs.input.bindAction("attack_simple", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::A});
    ecs.input.bindAction("attack_heavy", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Z});
    ecs.input.bindAction("special_attack", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::E});
    ecs.input.bindAction("block", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::R});

    _player2 = std::make_unique<Player>(ecs, std::pair<float, float>{700.f, 300.f}, 2);
    _player2->setSpriteAnimation("src/Smash/Content/player2-Sheet.png");
    _player2->flipXSprite(true);
    ecs.input.bindAction("move_left2", InputBinding{InputDeviceType::GamepadAxis, sf::Keyboard::Key::Left,
                                                    sf::Mouse::Button::Left, 0, 0, sf::Joystick::Axis::X, 50.f, -1});
    ecs.input.bindAction("move_right2", InputBinding{InputDeviceType::GamepadAxis, sf::Keyboard::Key::Right,
                                                     sf::Mouse::Button::Left, 0, 0, sf::Joystick::Axis::X, 50.f, 1});
    ecs.input.bindAction("jump2", InputBinding{InputDeviceType::GamepadAxis, sf::Keyboard::Key::Unknown,
                                               sf::Mouse::Button::Left, 0, 0, sf::Joystick::Axis::Y, 50.f, -1});
    ecs.input.bindAction("move_left2", InputBinding{InputDeviceType::GamepadAxis, sf::Keyboard::Key::Left,
                                                    sf::Mouse::Button::Left, 0, 0, sf::Joystick::Axis::PovX, 50.f, -1});
    ecs.input.bindAction("move_right2", InputBinding{InputDeviceType::GamepadAxis, sf::Keyboard::Key::Right,
                                                     sf::Mouse::Button::Left, 0, 0, sf::Joystick::Axis::PovX, 50.f, 1});
    ecs.input.bindAction("jump2", InputBinding{InputDeviceType::GamepadAxis, sf::Keyboard::Key::Unknown,
                                               sf::Mouse::Button::Left, 0, 0, sf::Joystick::Axis::PovY, 50.f, -1});
    ecs.input.bindAction("attack_simple2", InputBinding{InputDeviceType::GamepadButton, sf::Keyboard::Key::Unknown,
                                                        sf::Mouse::Button::Left, 0, 0});
    ecs.input.bindAction("attack_heavy2", InputBinding{InputDeviceType::GamepadButton, sf::Keyboard::Key::Unknown,
                                                       sf::Mouse::Button::Left, 0, 1});
    ecs.input.bindAction("special_attack2", InputBinding{InputDeviceType::GamepadButton, sf::Keyboard::Key::Unknown,
                                                         sf::Mouse::Button::Left, 0, 2});
    ecs.input.bindAction("block2", InputBinding{InputDeviceType::GamepadButton, sf::Keyboard::Key::Unknown,
                                                sf::Mouse::Button::Left, 0, 3});

    _UiPercentPlayer1 = ecs.registry.createEntity();
    _UiPercentPlayer2 = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _UiPercentPlayer1, TextComponent{"0%", "src/Smash/Content/font.otf", 40, sf::Color::Blue, 200, 700});
    ecs.registry.addComponent<TextComponent>(
        _UiPercentPlayer2, TextComponent{"0%", "src/Smash/Content/font.otf", 40, sf::Color::Red, 800, 700});

    _UiLivesPlayer1 = ecs.registry.createEntity();
    _UiLivesPlayer2 = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _UiLivesPlayer1, TextComponent{"3", "src/Smash/Content/font.otf", 20, sf::Color::Blue, 200, 750});
    ecs.registry.addComponent<TextComponent>(
        _UiLivesPlayer2, TextComponent{"3", "src/Smash/Content/font.otf", 20, sf::Color::Red, 800, 750});
    return;
}

void GameManager::update(ECS& ecs) {
    float dt = _deltaClock.restart().asSeconds();
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

    _player1->updateCooldown(dt);
    _player2->updateCooldown(dt);

    auto& input = ecs.input;

    this->handleCombat(ecs, *_player1, *_player2, "attack_simple", "attack", 5, 100.f, 1.0f);
    this->handleCombat(ecs, *_player1, *_player2, "attack_heavy", "attack_heavy", 8, 250.f, 2.0f);
    this->handleSpecialAttack(ecs, *_player1, "special_attack");

    this->handleCombat(ecs, *_player2, *_player1, "attack_simple2", "attack", 5, 100.f, 1.0f);
    this->handleCombat(ecs, *_player2, *_player1, "attack_heavy2", "attack_heavy", 8, 250.f, 2.0f);
    this->handleSpecialAttack(ecs, *_player2, "special_attack2");

    this->updateProjectiles(ecs, dt);

    if (this->checkEnd(ecs))
        return;
    this->updateUIHealthPercentage(ecs);
    return;
}

void GameManager::handleSpecialAttack(ECS& ecs, Player& attacker, const std::string& input) {
    if (ecs.input.isJustPressed(input) && attacker.canShoot()) {
        attacker.resetCooldown();

        auto& attackerSprite = ecs.registry.getComponent<AnimatedSprite2D>(attacker.getId());
        attackerSprite.playIfNotPlaying("special_attack");

        Entity projectile = ecs.registry.createEntity();
        auto& attackerPos = ecs.registry.getComponent<TransformComponent>(attacker.getId());

        bool facingLeft = attackerSprite.flipX;
        float direction = facingLeft ? -1.0f : 1.0f;
        float offsetX = facingLeft ? -50.0f : 50.0f;

        ecs.registry.addComponent<TransformComponent>(projectile, {attackerPos.x + offsetX, attackerPos.y});
        ecs.registry.addComponent<Velocity2D>(projectile, {direction * 300.0f, 0});
        ecs.registry.addComponent<SmashProjectileComponent>(projectile,
                                                            {attacker.getId(), 3.0f, 2.0f});  // 3 dmg, 2s lifetime

        AnimatedSprite2D anim;
        AnimationClip clip;
        clip.frames.push_back({64 * 22, 0, 64, 64});

        if (attackerSprite.animations.count("idle")) {
            clip.handle = attackerSprite.animations.at("idle").handle;
        }

        clip.mode = AnimationMode::Loop;
        clip.frameDuration = 0.1f;

        anim.animations.emplace("move", clip);
        anim.currentAnimation = "move";
        anim.flipX = facingLeft;

        ecs.registry.addComponent<AnimatedSprite2D>(projectile, anim);
    }
}

void GameManager::updateProjectiles(ECS& ecs, float dt) {
    auto& projectiles = ecs.registry.getEntities<SmashProjectileComponent>();
    std::vector<Entity> toDestroy;

    for (size_t i = 0; i < projectiles.size(); ++i) {
        Entity entityId = projectiles[i];
        if (!ecs.registry.hasComponent<SmashProjectileComponent>(entityId) ||
            !ecs.registry.hasComponent<TransformComponent>(entityId))
            continue;

        auto& proj = ecs.registry.getComponent<SmashProjectileComponent>(entityId);
        auto& pos = ecs.registry.getComponent<TransformComponent>(entityId);

        proj.lifetime -= dt;
        if (proj.lifetime <= 0) {
            toDestroy.push_back(entityId);
            continue;
        }

        Entity p1Id = _player1->getId();
        if (proj.ownerId != p1Id) {
            auto& p1Pos = ecs.registry.getComponent<TransformComponent>(p1Id);
            if (std::abs(pos.x - p1Pos.x) < 40 && std::abs(pos.y - p1Pos.y) < 40) {
                bool isBlocking = false;
                if (ecs.registry.hasComponent<AnimatedSprite2D>(p1Id)) {
                    auto& anim = ecs.registry.getComponent<AnimatedSprite2D>(p1Id);
                    if (anim.currentAnimation == "block")
                        isBlocking = true;
                }

                auto& health = ecs.registry.getComponent<PercentageHealth>(p1Id);
                float finalDamage = proj.damage;
                if (isBlocking)
                    finalDamage /= 2.0f;
                health.percent += finalDamage;

                if (!isBlocking || health.percent >= 150) {
                    auto& ejection = ecs.registry.getComponent<EjectionComponent>(p1Id);
                    float force = 50.0f + (health.percent * 0.1f);
                    float dir = (p1Pos.x > pos.x) ? 1.0f : -1.0f;
                    ejection.ejectionForce = {dir * force, -force};
                    ejection.ejected = true;
                    ejection.duration = 0.2f;

                    auto& sprite = ecs.registry.getComponent<AnimatedSprite2D>(p1Id);
                    sprite.playIfNotPlaying("take_damage");
                }

                toDestroy.push_back(entityId);
                continue;
            }
        }

        Entity p2Id = _player2->getId();
        if (proj.ownerId != p2Id) {
            auto& p2Pos = ecs.registry.getComponent<TransformComponent>(p2Id);
            if (std::abs(pos.x - p2Pos.x) < 40 && std::abs(pos.y - p2Pos.y) < 40) {
                bool isBlocking = false;
                if (ecs.registry.hasComponent<AnimatedSprite2D>(p2Id)) {
                    auto& anim = ecs.registry.getComponent<AnimatedSprite2D>(p2Id);
                    if (anim.currentAnimation == "block")
                        isBlocking = true;
                }

                auto& health = ecs.registry.getComponent<PercentageHealth>(p2Id);
                float finalDamage = proj.damage;
                if (isBlocking)
                    finalDamage /= 2.0f;
                health.percent += finalDamage;

                if (!isBlocking || health.percent >= 150) {
                    auto& ejection = ecs.registry.getComponent<EjectionComponent>(p2Id);
                    float force = 50.0f + (health.percent * 0.1f);
                    float dir = (p2Pos.x > pos.x) ? 1.0f : -1.0f;
                    ejection.ejectionForce = {dir * force, -force};
                    ejection.ejected = true;
                    ejection.duration = 0.2f;

                    auto& sprite = ecs.registry.getComponent<AnimatedSprite2D>(p2Id);
                    sprite.playIfNotPlaying("take_damage");
                }

                toDestroy.push_back(entityId);
                continue;
            }
        }
    }

    for (auto e : toDestroy) {
        ecs.registry.destroyEntity(e);
    }
}
void GameManager::handleCombat(ECS& ecs, Player& attacker, Player& victim, const std::string& attackInput,
                               const std::string& attackName, int damage, float forceBase, float forceScaling) {
    if (ecs.input.isJustPressed(attackInput)) {
        auto& posAttacker = ecs.registry.getComponent<TransformComponent>(attacker.getId());
        auto& posVictim = ecs.registry.getComponent<TransformComponent>(victim.getId());

        bool collision =
            (std::abs(posAttacker.x - posVictim.x) < 50.0f) && (std::abs(posAttacker.y - posVictim.y) < 50.0f);

        if (collision) {
            bool isBlocking = false;
            if (ecs.registry.hasComponent<AnimatedSprite2D>(victim.getId())) {
                auto& anim = ecs.registry.getComponent<AnimatedSprite2D>(victim.getId());
                if (anim.currentAnimation == "block")
                    isBlocking = true;
            }

            auto& victimHealth = ecs.registry.getComponent<PercentageHealth>(victim.getId());

            float finalDamage = damage;
            if (isBlocking)
                finalDamage /= 2.0f;

            victimHealth.percent += finalDamage;

            if (!isBlocking || victimHealth.percent >= 150) {
                auto& ejection = ecs.registry.getComponent<EjectionComponent>(victim.getId());
                float totalForce = forceBase + (victimHealth.percent * forceScaling);

                float direction = (posVictim.x > posAttacker.x) ? 1.0f : -1.0f;

                ejection.ejectionForce = {direction * totalForce, -totalForce};  // Up and away
                ejection.ejected = true;
                ejection.duration = 0.5f;

                auto& victimSprite = ecs.registry.getComponent<AnimatedSprite2D>(victim.getId());
                victimSprite.playIfNotPlaying("take_damage");
            }
        }
    }
}

void GameManager::loadInputSetting(ECS& ecs) {
    return;
}

// Private Methods

void GameManager::setBackground(ECS& ecs) {
    Entity background = ecs.registry.createEntity();

    Sprite2D sprite;
    std::string pathname = "src/Smash/Content/bg_smash.jpg";
    sprite.rect = {0, 0, 1920, 800};
    sprite.layer = RenderLayer::Background;
    if (ecs._textureManager.is_loaded(pathname)) {
        sprite.handle = ecs._textureManager.get_handle(pathname).value();
    } else {
        sprite.handle = ecs._textureManager.load(pathname, sf::Texture(pathname));
    }

    ecs.registry.addComponent<TransformComponent>(background, {500.f, 400.f});
    ecs.registry.addComponent<Sprite2D>(background, sprite);
    return;
}

void GameManager::setPlatform(ECS& ecs) {
    Entity platform = ecs.registry.createEntity();

    Sprite2D sprite;
    std::string pathname = "src/Smash/Content/platform.png";
    sprite.rect = {0, 0, 500, 100};
    sprite.layer = RenderLayer::Midground;
    if (ecs._textureManager.is_loaded(pathname)) {
        sprite.handle = ecs._textureManager.get_handle(pathname).value();
    } else {
        sprite.handle = ecs._textureManager.load(pathname, sf::Texture(pathname));
    }

    ecs.registry.addComponent<TransformComponent>(platform, {500.f, 600.f});
    ecs.registry.addComponent<GroundComponent>(platform, {{250, 550, 500, 100}, true});
    ecs.registry.addComponent<Sprite2D>(platform, sprite);
    return;
}

void GameManager::updateUIHealthPercentage(ECS& ecs) {
    if (_player1 && _player2) {
        auto health1 = _player1->getCurrentHealthPercentage();
        auto health2 = _player2->getCurrentHealthPercentage();
        ecs.registry.getComponent<TextComponent>(_UiPercentPlayer1).text =
            std::to_string(static_cast<int>(health1)) + "%";
        ecs.registry.getComponent<TextComponent>(_UiPercentPlayer2).text =
            std::to_string(static_cast<int>(health2)) + "%";
        ecs.registry.getComponent<TextComponent>(_UiLivesPlayer1).text =
            "lives: " + std::to_string(ecs.registry.getComponent<PercentageHealth>(_player1->getId()).totalLifeRespawn);
        ecs.registry.getComponent<TextComponent>(_UiLivesPlayer2).text =
            "lives: " + std::to_string(ecs.registry.getComponent<PercentageHealth>(_player2->getId()).totalLifeRespawn);
    }
}

bool GameManager::checkEnd(ECS& ecs) {
    if (_gameOver)
        return true;
    if (_player1->OutOfMap(-100.f, -100.f, 1100.f, 900.f)) {
        PercentageHealth& player1Health = ecs.registry.getComponent<PercentageHealth>(_player1->getId());
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
        ecs.registry.getComponent<TextComponent>(_UiLivesPlayer1).text =
            "lives: " + std::to_string(ecs.registry.getComponent<PercentageHealth>(_player1->getId()).totalLifeRespawn);
        ecs.registry.getComponent<TextComponent>(_UiLivesPlayer2).text =
            "lives: " + std::to_string(ecs.registry.getComponent<PercentageHealth>(_player2->getId()).totalLifeRespawn);
        return true;
    }
    if (_player2->OutOfMap(-100.f, -100.f, 1100.f, 900.f)) {
        PercentageHealth& player2Health = ecs.registry.getComponent<PercentageHealth>(_player2->getId());
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
        ecs.registry.getComponent<TextComponent>(_UiLivesPlayer1).text =
            "lives: " + std::to_string(ecs.registry.getComponent<PercentageHealth>(_player1->getId()).totalLifeRespawn);
        ecs.registry.getComponent<TextComponent>(_UiLivesPlayer2).text =
            "lives: " + std::to_string(ecs.registry.getComponent<PercentageHealth>(_player2->getId()).totalLifeRespawn);
        return true;
    }
    return false;
}
