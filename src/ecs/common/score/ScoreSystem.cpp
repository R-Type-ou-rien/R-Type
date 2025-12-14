/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ScoreSystem
*/

#include "ScoreSystem.hpp"

void ScoreSystem::update(Registry& registry, system_context context) {
    auto entities = registry.getEntities<AddScoreComponent>();

    for (auto entity : entities) {
        if (registry.hasComponent<ScoreComponent>(entity)) {
            auto& score = registry.getComponent<ScoreComponent>(entity);
            auto& addScore = registry.getComponent<AddScoreComponent>(entity);
            if (score.value < 0)
                score.value = 0;
        }
        registry.removeComponent<AddScoreComponent>(entity);
    }
}
