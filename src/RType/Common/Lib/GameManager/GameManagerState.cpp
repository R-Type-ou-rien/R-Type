#include <string>
#include <algorithm>
#include "GameManager.hpp"
#include "ECS.hpp"
#include "src/RType/Common/Systems/score.hpp"
#include "src/RType/Common/Systems/spawn.hpp"
#include "src/RType/Common/Systems/health.hpp"
#include "src/RType/Common/Components/status_display_components.hpp"
#include "src/RType/Common/Components/leaderboard_component.hpp"
#include "src/RType/Common/Components/spectator_component.hpp"
#include "src/RType/Common/Components/game_over_notification.hpp"
#include "Components/StandardComponents.hpp"
#include "src/RType/Common/Systems/leaderboard_system.hpp"

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

void GameManager::updateUI(Environment& env) {
    auto& ecs = env.getECS();

    if (env.isServer() || _gameOver || _victory) {
        return;
    }

    Entity player_id = (_player) ? _player->getId() : findPlayerEntity(ecs.registry);

    if (player_id != -1 && _statusDisplayEntity != static_cast<Entity>(-1)) {
        if (ecs.registry.hasComponent<StatusDisplayComponent>(_statusDisplayEntity)) {
            ecs.registry.getComponent<StatusDisplayComponent>(_statusDisplayEntity).player_entity = player_id;
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
                                       std::to_string(health.max_hp) + " (" + std::to_string(static_cast<int>(percent)) + "%)";
                    
                    if (percent > 50) boss_hp_text.color = sf::Color::Red;
                    else if (percent > 25) boss_hp_text.color = sf::Color(255, 165, 0);
                    else boss_hp_text.color = sf::Color::Magenta;
                    
                    found_boss = true;
                }
                break;
            }
        }
        if (!found_boss) boss_hp_text.text = "";
    }
}

void GameManager::checkGameState(Environment& env) {
    auto& ecs = env.getECS();

    if (!env.isServer()) {
        auto& gameOverEntities = ecs.registry.getEntities<GameOverNotification>();
        if (!gameOverEntities.empty()) {
            auto& notification = ecs.registry.getConstComponent<GameOverNotification>(gameOverEntities[0]);
            _gameOver = !notification.victory;
            _victory = notification.victory;
            
            ecs.registry.destroyEntity(gameOverEntities[0]);
            if (!_leaderboardDisplayed) {
                displayLeaderboard(env, notification.victory);
                _leaderboardDisplayed = true;
            }
            return;
        }
    } else if (!env.isClient()) {
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
    
    if (total_players == 1) {
        if (alive_players == 0) {
            _gameOver = true;
            
            if (env.isStandalone() && !_leaderboardDisplayed) {
                displayGameOver(env, false);
                displayLeaderboard(env, false);
                _leaderboardDisplayed = true;
            }
            return;
        }
    } else if (total_players >= 2) {
        for (auto entity : player_entities) {
            if (!ecs.registry.hasComponent<HealthComponent>(entity))
                continue;
                
            auto& health = ecs.registry.getComponent<HealthComponent>(entity);
            bool is_dead = health.current_hp <= 0;
            bool is_spectator = ecs.registry.hasComponent<SpectatorComponent>(entity);
            
            if (is_dead && !is_spectator && alive_players > 0) {
                SpectatorComponent spectator;
                
                for (auto other_entity : player_entities) {
                    if (other_entity != entity && ecs.registry.hasComponent<HealthComponent>(other_entity)) {
                        auto& other_health = ecs.registry.getConstComponent<HealthComponent>(other_entity);
                        if (other_health.current_hp > 0) {
                            spectator.watching_player = other_entity;
                            break;
                        }
                    }
                }
                
                ecs.registry.addComponent<SpectatorComponent>(entity, spectator);
                
                if (!env.isServer() && _player && _player->getId() == entity) {
                    Entity spectatorMsg = ecs.registry.createEntity();
                    ecs.registry.addComponent<TextComponent>(
                        spectatorMsg, 
                        {"SPECTATOR MODE - Watching other players...", 
                         "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 
                         32, sf::Color::Yellow, 600, 50});
                }
            }
        }
        
        if (alive_players == 0 && dead_players > 0) {
            _gameOver = true;
            
            if (env.isStandalone() && !_leaderboardDisplayed) {
                displayGameOver(env, false);
                displayLeaderboard(env, false);
                _leaderboardDisplayed = true;
            }
            return;
        }
    }

    bool boss_exists = false;
    bool boss_spawned = false;

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

    // IMPORTANT: Vérifier si le boss existe seulement s'il a été spawné
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

        // VICTOIRE seulement si : boss a été spawné ET boss n'existe plus ET au moins 1 joueur vivant
        if (!boss_exists && alive_players > 0) {
            _victory = true;
            
            // Afficher WIN à tous les joueurs
            if (!_leaderboardDisplayed) {
                displayGameOver(env, true);
                displayLeaderboard(env, true);
                _leaderboardDisplayed = true;
            }
        }
    }
}

void GameManager::displayGameOver(Environment& env, bool victory) {
    auto& ecs = env.getECS();
    std::string message;
    sf::Color color;

    if (env.isServer()){
        return;
    }

    _gameStateEntity = ecs.registry.createEntity();

    ecs.registry.addComponent<TextComponent>(_gameStateEntity, {message, "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 72, color, 700, 350});

    if (victory == true) {
        message = "Victory";
        color = sf::Color::Green;
    } else {
        message = "Game over";
        color = sf::Color::Red;

    }
}

void GameManager::displayLeaderboard(Environment& env, bool victory) {
    auto& ecs = env.getECS();

    if (env.isServer()) {
        return;
    }
    _leaderboardEntity = LeaderboardSystem::createLeaderboard(ecs.registry, victory);
}
