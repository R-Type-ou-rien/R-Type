#include <string>
#include <algorithm>
#include <vector>
#include "GameManager.hpp"
#include "ECS.hpp"

#include "src/RType/Common/Systems/score.hpp"
#include "src/RType/Common/Systems/health.hpp"
#include "src/RType/Common/Components/status_display_components.hpp"
#include "src/RType/Common/Components/leaderboard_component.hpp"
#include "src/RType/Common/Components/game_over_notification.hpp"
#include "Components/StandardComponents.hpp"
#include "src/RType/Common/Components/LevelTransitionComponent.hpp"
#include "src/RType/Common/Systems/leaderboard_system.hpp"

#if defined(SERVER_BUILD)
#include "src/Engine/Lib/Components/NetworkComponents.hpp"
#endif

static Entity findPlayerEntity(Registry& registry) {
    auto& entities = registry.getEntities<TagComponent>();
    for (auto entity : entities) {
        auto& tags = registry.getConstComponent<TagComponent>(entity);
        for (const auto& tag : tags.tags) {
            if (tag == "PLAYER") {
                return entity;
            }
        }
    }
    return -1;
}

void GameManager::updateUI(std::shared_ptr<Environment> env) {
    if (_previousState != Environment::GameState::IN_GAME) {
        initUI(env);
        _previousState = Environment::GameState::IN_GAME;
    }
    auto& ecs = env->getECS();

    if (env->isServer() || _gameOver || _victory) {
        return;
    }

    Entity player_id = (_player) ? _player->getId() : findPlayerEntity(ecs.registry);

    // On Client, let StatusDisplaySystem handle the connection to the correct player entity (via NetworkIdentity)
    // On Server/Standalone, we can assign it here.
    if (!env->isClient() && player_id != -1 && _statusDisplayEntity != static_cast<Entity>(-1)) {
        if (ecs.registry.hasComponent<StatusDisplayComponent>(_statusDisplayEntity)) {
            ecs.registry.getComponent<StatusDisplayComponent>(_statusDisplayEntity).setPlayerEntity(player_id);
        }
    }

    if (_timerEntity != static_cast<Entity>(-1) && ecs.registry.hasComponent<TextComponent>(_timerEntity)) {
        auto& game_timers = ecs.registry.getEntities<GameTimerComponent>();
        if (!game_timers.empty()) {
            auto& timer = ecs.registry.getComponent<GameTimerComponent>(game_timers[0]);
            ecs.registry.getComponent<TextComponent>(_timerEntity).text =
                "Time: " + std::to_string(static_cast<int>(timer.elapsed_time)) + "s";
        }
    }

    if (_bossHPEntity != static_cast<Entity>(-1) && ecs.registry.hasComponent<TextComponent>(_bossHPEntity)) {
        auto& boss_hp_text = ecs.registry.getComponent<TextComponent>(_bossHPEntity);
        bool found_boss = false;

        auto& entities = ecs.registry.getEntities<TagComponent>();

        for (auto entity : entities) {
            auto& tags = ecs.registry.getConstComponent<TagComponent>(entity);
            if (std::find(tags.tags.begin(), tags.tags.end(), "BOSS") != tags.tags.end()) {
                if (ecs.registry.hasComponent<HealthComponent>(entity)) {
                    auto& health = ecs.registry.getComponent<HealthComponent>(entity);
                    float percent = (static_cast<float>(health.current_hp) / health.max_hp) * 100.0f;
                    boss_hp_text.text = "BOSS: " + std::to_string(health.current_hp) + "/" +
                                        std::to_string(health.max_hp) + " (" +
                                        std::to_string(static_cast<int>(percent)) + "%)";

                    if (percent > 50)
                        boss_hp_text.color = sf::Color::Red;
                    else if (percent > 25)
                        boss_hp_text.color = sf::Color(255, 165, 0);
                    else
                        boss_hp_text.color = sf::Color::Magenta;

                    found_boss = true;
                }
                break;
            }
        }
        if (!found_boss)
            boss_hp_text.text = "";
    }
}

void GameManager::checkGameState(std::shared_ptr<Environment> env) {
    auto& ecs = env->getECS();

    if (!env->isServer()) {
        auto& gameOverEntities = ecs.registry.getEntities<GameOverNotification>();
        if (!gameOverEntities.empty()) {
            auto& notification = ecs.registry.getConstComponent<GameOverNotification>(gameOverEntities[0]);
            _gameOver = !notification.victory;
            _victory = notification.victory;

            ecs.registry.destroyEntity(gameOverEntities[0]);
        }
    } else if (!env->isClient()) {
        // Server/standalone: if we expected a player entity but it is missing, it's game over.
        if (_player) {
            _gameOver = true;
            displayGameOver(env, false);
            return;
        }
    }

    if (_gameOver || _victory) {
        if (!_leaderboardDisplayed) {
            displayLeaderboard(env, _victory);
            _leaderboardDisplayed = true;
            _nextLevelLoaded = false;

            if (_victory && !env->isClient()) {
                auto& spawners = ecs.registry.getEntities<EnemySpawnComponent>();
                for (auto spawner : spawners) {
                    if (!ecs.registry.hasComponent<EnemySpawnComponent>(spawner))
                        continue;
                    ecs.registry.getComponent<EnemySpawnComponent>(spawner).is_active = false;
                }

                auto& pod_spawners = ecs.registry.getEntities<PodSpawnComponent>();
                for (auto spawner : pod_spawners) {
                    if (!ecs.registry.hasComponent<PodSpawnComponent>(spawner))
                        continue;
                    ecs.registry.getComponent<PodSpawnComponent>(spawner).can_spawn = false;
                }
            }

            if (_victory) {
                _inTransition = true;
            }
        } else if (_victory) {
            auto& leaderboard_entities = ecs.registry.getEntities<LeaderboardComponent>();
            if (leaderboard_entities.empty() && !_nextLevelLoaded) {
                if (!env->isClient()) {
                    _nextLevelLoaded = true;
                    loadNextLevel(env);
                } else {
                    _nextLevelLoaded = true;
                }
            }

            if (env->isClient() && leaderboard_entities.empty() && _leaderboardDisplayed) {
                _victory = false;
                _leaderboardDisplayed = false;
                _inTransition = false;
                _nextLevelLoaded = false;
                _gameOver = false;
            }
        }
        return;
    }

    int total_players = 0;
    int alive_players = 0;
    int dead_players = 0;
    std::vector<Entity> player_entities;

    auto& entities = ecs.registry.getEntities<TagComponent>();
    for (auto entity : entities) {
        auto& tags = ecs.registry.getConstComponent<TagComponent>(entity);
        for (const auto& tag : tags.tags) {
            if (tag == "PLAYER") {
                total_players++;
                player_entities.push_back(entity);

                bool is_alive = false;
                if (ecs.registry.hasComponent<HealthComponent>(entity)) {
                    auto& health = ecs.registry.getConstComponent<HealthComponent>(entity);
                    is_alive = health.current_hp > 0;
                }

                if (is_alive) {
                    alive_players++;
                } else {
                    dead_players++;
                }
                break;
            }
        }
    }

    if (total_players > 0 && alive_players == 0) {
        if (!env->isClient()) {
            _gameOver = true;
        }

        if (env->isStandalone() && !_leaderboardDisplayed) {
            displayLeaderboard(env, false);
            _leaderboardDisplayed = true;
        }

#if defined(SERVER_BUILD)
        if (env->isServer() && !_leaderboardDisplayed) {
            if (env->hasFunction("broadcastGameOver")) {
                std::vector<std::tuple<uint32_t, int, bool>> scores;
                for (auto pEntity : player_entities) {
                    uint32_t client_id = 0;
                    int score_val = 0;
                    bool is_alive = false;

                    if (ecs.registry.hasComponent<NetworkIdentity>(pEntity)) {
                        client_id = ecs.registry.getConstComponent<NetworkIdentity>(pEntity).ownerId;
                    }
                    if (ecs.registry.hasComponent<ScoreComponent>(pEntity)) {
                        score_val = ecs.registry.getConstComponent<ScoreComponent>(pEntity).current_score;
                    }
                    if (ecs.registry.hasComponent<HealthComponent>(pEntity)) {
                        is_alive = ecs.registry.getConstComponent<HealthComponent>(pEntity).current_hp > 0;
                    }
                    scores.emplace_back(client_id, score_val, is_alive);
                }

                auto func = env->getFunction<
                    std::function<void(uint32_t, bool, const std::vector<std::tuple<uint32_t, int, bool>>&)>>(
                    "broadcastGameOver");
                // Assuming _currentLobbyId is valid
                func(_currentLobbyId, false, scores);
                _leaderboardDisplayed = true;
                std::cout << "[GameManagerState] Game Over broadcasted (All " << total_players << " players dead)"
                          << std::endl;
            }
        }
#endif
        return;
    }

    bool boss_exists = false;
    bool boss_spawned = false;

    auto& spawners = ecs.registry.getEntities<EnemySpawnComponent>();
    for (auto spawner : spawners) {
        if (ecs.registry.hasComponent<EnemySpawnComponent>(spawner)) {
            if (ecs.registry.getConstComponent<EnemySpawnComponent>(spawner).boss_arrived) {
                boss_spawned = true;
                break;
            }
        }
    }

    if (boss_spawned && !_inTransition) {
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

        if (!boss_exists && alive_players > 0 && !_victory) {
            _victory = true;
            if (!_leaderboardDisplayed) {
                displayLeaderboard(env, true);
                _leaderboardDisplayed = true;
            }
            // Transition logic for standalone...
            Entity transitionEntity = ecs.registry.createEntity();
            LevelTransitionComponent transition;
            transition.state = LevelTransitionComponent::TransitionState::IDLE;
            transition.next_level_name = "src/RType/Common/content/config/level2_spawns.cfg";
            ecs.registry.addComponent<LevelTransitionComponent>(transitionEntity, transition);
        }
    }
}

void GameManager::displayGameOver(std::shared_ptr<Environment> env, bool victory) {
    auto& ecs = env->getECS();
    std::string message;
    sf::Color color;

    if (env->isServer()) {
        return;
    }

    if (victory == true) {
        message = "Victory";
        color = sf::Color::Green;
    } else {
        message = "Game over";
        color = sf::Color::Red;
    }

    _gameStateEntity = ecs.registry.createEntity();
    {
        TagComponent tag;
        tag.tags.push_back("LEADERBOARD");
        ecs.registry.addComponent<TagComponent>(_gameStateEntity, tag);
    }
    ecs.registry.addComponent<TextComponent>(
        _gameStateEntity,
        {message, "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 72, color, 700, 350});

    // Message secondaire
    Entity subMessage = ecs.registry.createEntity();
    std::string subText = victory ? "Congratulations!" : "Better luck next time!";
    ecs.registry.addComponent<TextComponent>(
        subMessage,
        {subText, "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 28, sf::Color::White, 750, 430});
}

void GameManager::displayLeaderboard(std::shared_ptr<Environment> env, bool victory) {
    auto& ecs = env->getECS();

    if (env->isServer()) {
        return;
    }
    _leaderboardEntity = LeaderboardSystem::createLeaderboard(ecs.registry, victory);
}
