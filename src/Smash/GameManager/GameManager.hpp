/*
** EPITECH PROJECT, 2025
** Smash
** File description:
** GameManager.hpp
*/

#pragma once

#include <memory>
#include <SFML/System/Clock.hpp>
#include "../Lib/EjectionSystem.hpp"
#include "../Lib/ProjectileComponent.hpp"
#include "../Player/Player.hpp"
#include "../../../Engine/Core/ECS/ECS.hpp"

class GameManager {
public:
    GameManager();
    void init(ECS& ecs);
    void update(ECS& ecs);
    void loadInputSetting(ECS& ecs);

private:
    void setPlatform(ECS& ecs);
    void setBackground(ECS& ecs);
    void updateUIHealthPercentage(ECS& ecs);
    void handleCombat(ECS& ecs, Player& attacker, Player& victim, const std::string& input, const std::string& animationName, int damage, float forceBase, float forceScaling);
    void handleSpecialAttack(ECS& ecs, Player& attacker, const std::string& input);
    void updateProjectiles(ECS& ecs, float dt);
    bool checkEnd(ECS& ecs);
private:
    sf::Clock _deltaClock;
    std::unique_ptr<Player> _player1;
    std::unique_ptr<Player> _player2;
    Entity _UiPercentPlayer1;
    Entity _UiPercentPlayer2;
    Entity _UiLivesPlayer1;
    Entity _UiLivesPlayer2;
    bool _gameOver = false;
};
