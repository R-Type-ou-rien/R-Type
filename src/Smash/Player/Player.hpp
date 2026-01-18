/*
** EPITECH PROJECT, 2025
** Smash
** File description:
** Player.hpp
*/

#pragma once

#include "../../../Engine/Core/ECS/ECS.hpp"
#include "../../../Engine/Core/ECS/EcsType.hpp"

class Player {
    public:
        Player(ECS& ecs, std::pair<float, float> pos, int numberPlayer);

        void setSpriteAnimation(const std::string pathname);
        void flipXSprite(bool flip);
        void setPosition(std::pair<float, float> pos);
        void reset();
        void resetGravity();
        void resetVelocity();
        void resetAnimation();

        void checkMove();
        void updateAnimation();

        bool OutOfMap(float xMin, float yMin, float xMax, float yMax);

        void setPlayable(bool playable);

        Entity getId() const;
        float getCurrentHealthPercentage();
    private:
        int _numberPlayer;
        std::pair<float, float> _startPos;
        float _gravityAccumulate = 0.0f;
        ECS& _ecs;
        Entity _id;
        bool _playable = true;
};