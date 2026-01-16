#include <string>
#include "GameManager.hpp"
#include "ECS.hpp"
#include "src/RType/Common/Systems/score.hpp"
#include "src/RType/Common/Systems/spawn.hpp"
#include "src/RType/Common/Systems/health.hpp"

void GameManager::updateUI(Environment& env) {
    auto& ecs = env.getECS();

    if (!env.isServer()) {
        if (_gameOver || _victory) {
            return;
        }

        // Update HP
        if (ecs.registry.hasComponent<TextComponent>(_uiEntity)) {
            auto& text = ecs.registry.getComponent<TextComponent>(_uiEntity);

            if (_player) {
                Entity player_id = _player->getId();
                if (ecs.registry.hasComponent<HealthComponent>(player_id)) {
                    int hp = _player->getCurrentHealth();
                    int max_hp = _player->getMaxHealth();
                    text.text = "HP: " + std::to_string(hp) + "/" + std::to_string(max_hp);
                }
            }
        }

        // Update Score
        if (ecs.registry.hasComponent<TextComponent>(_scoreEntity)) {
            auto& score_text = ecs.registry.getComponent<TextComponent>(_scoreEntity);
            int score = ScoreSystem::getScore(ecs.registry);
            score_text.text = "Score: " + std::to_string(score);
        }
    }
}

void GameManager::checkGameState(Environment& env) {
    auto& ecs = env.getECS();

    if (_gameOver || _victory) {
        return;
    }

    // Vérification de la défaite (joueur mort)
    if (_player) {
        Entity player_id = _player->getId();
        if (!ecs.registry.hasComponent<HealthComponent>(player_id)) {
            _gameOver = true;
            displayGameOver(env, false);
            return;
        }
        auto& health = ecs.registry.getConstComponent<HealthComponent>(player_id);
        if (health.current_hp <= 0) {
            _gameOver = true;
            displayGameOver(env, false);
            return;
        }
    } else {
        _gameOver = true;
        displayGameOver(env, false);
        return;
    }

    // Vérification de la victoire (boss tué)
    auto& entities = ecs.registry.getEntities<TagComponent>();
    bool boss_exists = false;
    bool boss_spawned = false;

    // Vérifier si le boss a été spawné
    auto& spawners = ecs.registry.getEntities<EnemySpawnComponent>();
    for (auto spawner : spawners) {
        if (ecs.registry.hasComponent<EnemySpawnComponent>(spawner)) {
            auto& spawn_comp = ecs.registry.getConstComponent<EnemySpawnComponent>(spawner);
            if (spawn_comp.boss_arrived) {
                boss_spawned = true;
                break;
            }
        }
    }

    if (boss_spawned) {
        for (auto entity : entities) {
            auto& tags = ecs.registry.getConstComponent<TagComponent>(entity);
            for (const auto& tag : tags.tags) {
                if (tag == "BOSS") {
                    boss_exists = true;
                    break;
                }
            }
            if (boss_exists)
                break;
        }

        if (!boss_exists) {
            _victory = true;
            displayGameOver(env, true);
        }
    }
}

void GameManager::displayGameOver(Environment& env, bool victory) {
    auto& ecs = env.getECS();

    if (!env.isServer()) {
        _gameStateEntity = ecs.registry.createEntity();

        std::string message = victory ? "VICTORY!" : "GAME OVER";
        sf::Color color = victory ? sf::Color::Green : sf::Color::Red;

        ecs.registry.addComponent<TextComponent>(
            _gameStateEntity, {message, "content/open_dyslexic/OpenDyslexic-Regular.otf", 72, color, 700, 450});

        // Afficher le score final
        Entity scoreMessage = ecs.registry.createEntity();
        int final_score = ScoreSystem::getScore(ecs.registry);
        std::string scoreText = "Final Score: " + std::to_string(final_score);
        ecs.registry.addComponent<TextComponent>(
            scoreMessage,
            {scoreText, "content/open_dyslexic/OpenDyslexic-Regular.otf", 36, sf::Color::Yellow, 750, 550});

        // Message secondaire
        Entity subMessage = ecs.registry.createEntity();
        std::string subText = victory ? "Congratulations!" : "Try Again...";
        ecs.registry.addComponent<TextComponent>(
            subMessage, {subText, "content/open_dyslexic/OpenDyslexic-Regular.otf", 28, sf::Color::White, 800, 620});
    }
}
