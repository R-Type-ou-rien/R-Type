#include "score.hpp"
#include "Components/StandardComponents.hpp"
#include <iostream>

void ScoreSystem::update(Registry& registry, system_context context) {
    // Le score est mis à jour ailleurs (quand un ennemi meurt)
}

void ScoreSystem::addScore(Registry& registry, int points) {
    auto& scoreEntities = registry.getEntities<ScoreComponent>();
    if (scoreEntities.empty()) {
        std::cerr << "[ScoreSystem] ERROR: No ScoreComponent entity found!" << std::endl;
        return;
    }

    for (auto entity : scoreEntities) {
        auto& score = registry.getComponent<ScoreComponent>(entity);
        int old_score = score.current_score;
        score.current_score += points;
        if (score.current_score > score.high_score) {
            score.high_score = score.current_score;
        }
        std::cout << "[ScoreSystem] Added " << points << " points. Score: " << old_score << " -> "
                  << score.current_score << std::endl;
        break;
    }
}

int ScoreSystem::getScore(Registry& registry) {
    auto& scoreEntities = registry.getEntities<ScoreComponent>();
    for (auto entity : scoreEntities) {
        auto& score = registry.getConstComponent<ScoreComponent>(entity);
        return score.current_score;
    }
    return 0;
}
