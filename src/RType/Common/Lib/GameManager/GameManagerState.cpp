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

        // Update Timer
        if (ecs.registry.hasComponent<TextComponent>(_timerEntity)) {
            auto& timer_text = ecs.registry.getComponent<TextComponent>(_timerEntity);
            auto& game_timers = ecs.registry.getEntities<GameTimerComponent>();
            if (!game_timers.empty()) {
                auto& timer = ecs.registry.getComponent<GameTimerComponent>(game_timers[0]);
                int seconds = static_cast<int>(timer.elapsed_time);
                timer_text.text = "Time: " + std::to_string(seconds) + "s";
            }
        }

        // Update Boss HP (nouveau)
        if (ecs.registry.hasComponent<TextComponent>(_bossHPEntity)) {
            auto& boss_hp_text = ecs.registry.getComponent<TextComponent>(_bossHPEntity);
            
            // Chercher le boss
            auto& entities = ecs.registry.getEntities<TagComponent>();
            bool found_boss = false;
            
            for (auto entity : entities) {
                auto& tags = ecs.registry.getConstComponent<TagComponent>(entity);
                for (const auto& tag : tags.tags) {
                    if (tag == "BOSS") {
                        if (ecs.registry.hasComponent<HealthComponent>(entity)) {
                            auto& boss_health = ecs.registry.getComponent<HealthComponent>(entity);
                            int current_hp = boss_health.current_hp;
                            int max_hp = boss_health.max_hp;
                            float hp_percent = (static_cast<float>(current_hp) / max_hp) * 100.0f;
                            
                            boss_hp_text.text = "BOSS: " + std::to_string(current_hp) + "/" + std::to_string(max_hp) + " (" + std::to_string(static_cast<int>(hp_percent)) + "%)";
                            
                            // Changer la couleur selon la santé
                            if (hp_percent > 50) {
                                boss_hp_text.color = sf::Color::Red;
                            } else if (hp_percent > 25) {
                                boss_hp_text.color = sf::Color(255, 165, 0); // Orange
                            } else {
                                boss_hp_text.color = sf::Color(255, 0, 255); // Magenta (critique)
                            }
                            
                            found_boss = true;
                        }
                        break;
                    }
                }
                if (found_boss) break;
            }
            
            // Cacher le texte si pas de boss
            if (!found_boss) {
                boss_hp_text.text = "";
            }
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
        if (health.current_hp < 1) {
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
            _gameStateEntity,
            {message, "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 72, color, 700, 450});

        // Afficher le score final
        Entity scoreMessage = ecs.registry.createEntity();
        int final_score = ScoreSystem::getScore(ecs.registry);
        std::string scoreText = "Final Score: " + std::to_string(final_score);
        ecs.registry.addComponent<TextComponent>(
            scoreMessage, {scoreText, "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 36,
                           sf::Color::Yellow, 750, 550});

        // Message secondaire
        Entity subMessage = ecs.registry.createEntity();
        std::string subText = victory ? "Congratulations!" : "Try Again...";
        ecs.registry.addComponent<TextComponent>(
            subMessage, {subText, "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 28,
                         sf::Color::White, 800, 620});
    }
}
