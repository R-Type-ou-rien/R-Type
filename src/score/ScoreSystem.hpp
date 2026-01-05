/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ScoreSystem
*/

#ifndef SCORESYSTEM_HPP_
#define SCORESYSTEM_HPP_

#include "../../../Engine/Core/ECS/ISystem.hpp"
#include "../../../Engine/Core/ECS/Registry/registry.hpp"

struct ScoreComponent {
    int value = 0;
};

struct AddScoreComponent {
    int points = 0;
};

class ScoreSystem : public ISystem {
   public:
    ScoreSystem() = default;
    ~ScoreSystem() = default;

    void update(Registry& registry, system_context context) override;

   private:
};

#endif /* !SCORESYSTEM_HPP_ */
