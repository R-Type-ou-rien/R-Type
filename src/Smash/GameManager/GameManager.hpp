/*
** EPITECH PROJECT, 2025
** Smash
** File description:
** GameManager.hpp
*/

#pragma once

#include <memory>
#include <SFML/System/Clock.hpp>
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
    bool checkEnd(ECS& ecs);
private:
    std::unique_ptr<Player> _player1;
    std::unique_ptr<Player> _player2;
    Entity _UiPercentPlayer1;
    Entity _UiPercentPlayer2;
    Entity _UiLivesPlayer1;
    Entity _UiLivesPlayer2;
    bool _gameOver = false;
};
