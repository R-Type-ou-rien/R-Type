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

    if (!env.isServer()) {
        if (_gameOver || _victory) {
            return;
        }

        Entity player_id = -1;
        if (_player) {
            player_id = _player->getId();
        } else {
            player_id = findPlayerEntity(ecs.registry);
        }

        // Update Status Display player reference (only if local player exists)
        if (player_id != -1 && _statusDisplayEntity != static_cast<Entity>(-1) &&
            ecs.registry.hasComponent<StatusDisplayComponent>(_statusDisplayEntity)) {
            auto& status = ecs.registry.getComponent<StatusDisplayComponent>(_statusDisplayEntity);
            status.player_entity = player_id;
        }

        // Update Timer from server-synced GameTimerComponent
        if (_timerEntity != static_cast<Entity>(-1) && ecs.registry.hasComponent<TextComponent>(_timerEntity)) {
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

    // CLIENT SEULEMENT : Vérifier si on a reçu S_GAME_OVER du serveur
    if (!env.isServer()) {
        auto& gameOverEntities = ecs.registry.getEntities<GameOverNotification>();
        if (!gameOverEntities.empty()) {
            auto& notification = ecs.registry.getConstComponent<GameOverNotification>(gameOverEntities[0]);
            _gameOver = !notification.victory;
            _victory = notification.victory;
            
            // Supprimer le composant pour ne pas le traiter plusieurs fois
            ecs.registry.destroyEntity(gameOverEntities[0]);
            displayGameOver(env, notification.victory);
            if (!_leaderboardDisplayed) {
                displayLeaderboard(env, notification.victory);
                _leaderboardDisplayed = true;
            }
            return;
        }
    } else if (!env.isClient()) {
        // Server/standalone: if we expected a player entity but it is missing, it's game over.
        // (On client, we might just be waiting for spawn.)
        if (_player) {
            _gameOver = true;
            displayGameOver(env, false);
            return;
        }
    }

    if (_gameOver || _victory) {
        // Si le leaderboard n'a pas encore été affiché, l'afficher maintenant
        if (!_leaderboardDisplayed) {
            displayLeaderboard(env, _victory);
            _leaderboardDisplayed = true;
        }
        return;
    }

    // Compter le nombre de joueurs vivants et morts
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

        // Si le boss est tué, victoire (seulement s'il reste au moins 1 joueur vivant)
        if (!boss_exists) {
            if (alive_players <= 0) {
                _gameOver = true;
                displayGameOver(env, false);
                return;
            }

            _victory = true;
            
            // Afficher WIN à tous les joueurs
            displayGameOver(env, true);
            
            // Préparer la transition vers le niveau suivant
            if (!env.isServer()) {
                Entity transitionEntity = ecs.registry.createEntity();
                LevelTransitionComponent transition;
                transition.state = LevelTransitionComponent::TransitionState::IDLE;
                transition.next_level_name = "Level 2";  // À adapter selon votre système
                ecs.registry.addComponent<LevelTransitionComponent>(transitionEntity, transition);
            }
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
            {message, "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 72, color, 700, 350});

        // Message secondaire
        Entity subMessage = ecs.registry.createEntity();
        std::string subText = victory ? "Congratulations!" : "Better luck next time!";
        ecs.registry.addComponent<TextComponent>(
            subMessage, {subText, "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 28,
                         sf::Color::White, 750, 430});
    }
}

void GameManager::displayLeaderboard(Environment& env, bool victory) {
    auto& ecs = env.getECS();

    if (env.isServer()) {
        return;
    }

    // Collecter tous les joueurs et leurs scores
    std::vector<PlayerScoreEntry> player_scores;
    
    // Priorité aux données du leaderboard envoyées par le serveur
    auto& leaderboard_entities = ecs.registry.getEntities<TagComponent>();
    bool has_leaderboard_data = false;
    
    for (auto entity : leaderboard_entities) {
        if (!ecs.registry.hasComponent<TagComponent>(entity)) continue;
        
        auto& tags = ecs.registry.getConstComponent<TagComponent>(entity);
        
        bool is_leaderboard_data = false;
        
        for (const auto& tag : tags.tags) {
            if (tag == "LEADERBOARD_DATA") {
                is_leaderboard_data = true;
                has_leaderboard_data = true;
                break;
            }
        }
        
        if (is_leaderboard_data) {
            PlayerScoreEntry entry;
            entry.player_entity = entity;
            
            // Récupérer le vrai client_id depuis NetworkIdentity
            if (ecs.registry.hasComponent<NetworkIdentity>(entity)) {
                auto& net_id = ecs.registry.getConstComponent<NetworkIdentity>(entity);
                entry.player_name = "Player " + std::to_string(net_id.ownerId);
            } else {
                entry.player_name = "Player " + std::to_string(player_scores.size() + 1);
            }
            
            entry.score = 0;
            entry.is_alive = false;

            // Récupérer le score
            if (ecs.registry.hasComponent<ScoreComponent>(entity)) {
                auto& score_comp = ecs.registry.getConstComponent<ScoreComponent>(entity);
                entry.score = score_comp.current_score;
            }

            // Vérifier si vivant
            if (ecs.registry.hasComponent<HealthComponent>(entity)) {
                auto& health = ecs.registry.getConstComponent<HealthComponent>(entity);
                entry.is_alive = health.current_hp > 0;
            }

            player_scores.push_back(entry);
        }
    }
    
    // Si pas de données leaderboard, chercher les joueurs locaux (mode standalone)
    if (!has_leaderboard_data) {
        auto& entities = ecs.registry.getEntities<TagComponent>();
        
        for (auto entity : entities) {
            if (!ecs.registry.hasComponent<TagComponent>(entity)) {
                continue;
            }
            
            auto& tags = ecs.registry.getConstComponent<TagComponent>(entity);
            
            for (const auto& tag : tags.tags) {
                if (tag == "PLAYER") {
                    PlayerScoreEntry entry;
                    entry.player_entity = entity;
                    entry.player_name = "Player " + std::to_string(entity);
                    entry.score = 0;
                    entry.is_alive = false;

                    // Récupérer le score
                    if (ecs.registry.hasComponent<ScoreComponent>(entity)) {
                        auto& score_comp = ecs.registry.getConstComponent<ScoreComponent>(entity);
                        entry.score = score_comp.current_score;
                    }

                    // Vérifier si vivant
                    if (ecs.registry.hasComponent<HealthComponent>(entity)) {
                        auto& health = ecs.registry.getConstComponent<HealthComponent>(entity);
                        entry.is_alive = health.current_hp > 0;
                    }

                    player_scores.push_back(entry);
                    break;
                }
            }
        }
    }

    // S'il n'y a qu'un joueur, utiliser le score global
    if (player_scores.size() <= 1) {
        int global_score = ScoreSystem::getScore(ecs.registry);
        
        if (player_scores.empty()) {
            PlayerScoreEntry entry;
            entry.player_entity = -1;
            entry.player_name = "Player";
            entry.score = global_score;
            entry.is_alive = false;
            player_scores.push_back(entry);
        } else {
            player_scores[0].score = global_score;
        }
    }

    // Trier par score (décroissant)
    std::sort(player_scores.begin(), player_scores.end(), 
              [](const PlayerScoreEntry& a, const PlayerScoreEntry& b) {
                  return a.score > b.score;
              });

    // Calculer le score total combiné
    int total_combined_score = 0;
    for (const auto& entry : player_scores) {
        total_combined_score += entry.score;
    }

    // Créer le leaderboard
    _leaderboardEntity = ecs.registry.createEntity();
    LeaderboardComponent leaderboard;
    leaderboard.entries = player_scores;
    leaderboard.is_displayed = true;
    ecs.registry.addComponent<LeaderboardComponent>(_leaderboardEntity, leaderboard);

    // Titre du leaderboard
    Entity title = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        title, {"LEADERBOARD", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 48,
                sf::Color::Cyan, 750, 500});
    
    // Afficher le score total combiné (si mode multijoueur)
    float y_offset = 570;
    if (player_scores.size() > 1) {
        Entity totalScoreEntity = ecs.registry.createEntity();
        std::string total_text = "SCORE TOTAL: " + std::to_string(total_combined_score) + " pts";
        ecs.registry.addComponent<TextComponent>(
            totalScoreEntity, {total_text, "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 36,
                              sf::Color(100, 255, 100), 650, y_offset});  // Vert clair
        y_offset += 60;
        
        // Ligne de séparation
        Entity separatorEntity = ecs.registry.createEntity();
        ecs.registry.addComponent<TextComponent>(
            separatorEntity, {"─────────────────────────────", 
                            "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 24,
                            sf::Color(100, 100, 100), 620, y_offset});
        y_offset += 50;
    }
    
    // Afficher chaque entrée (classement individuel)
    int rank = 1;
    for (const auto& entry : player_scores) {
        Entity scoreEntity = ecs.registry.createEntity();
        
        std::string rank_text = "#" + std::to_string(rank) + "  ";
        std::string name_text = entry.player_name + ": ";
        std::string score_text = std::to_string(entry.score) + " pts";
        std::string status_text = entry.is_alive ? " ✓" : " ✗";
        
        std::string full_text = rank_text + name_text + score_text + status_text;
        
        sf::Color entry_color = sf::Color::White;
        if (rank == 1 && player_scores.size() > 1) {
            entry_color = sf::Color::Yellow; // 1er en or
        } else if (rank == 2) {
            entry_color = sf::Color(192, 192, 192); // 2ème en argent
        } else if (rank == 3) {
            entry_color = sf::Color(205, 127, 50); // 3ème en bronze
        }
        
        ecs.registry.addComponent<TextComponent>(
            scoreEntity, {full_text, "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 32,
                          entry_color, 650, y_offset});
        
        y_offset += 50;
        rank++;
    }

    // Message de fin
    Entity finalMsg = ecs.registry.createEntity();
    std::string final_text = victory ? "Press ESC to quit" : "Press ESC to quit";
    ecs.registry.addComponent<TextComponent>(
        finalMsg, {final_text, "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 24,
                   sf::Color(150, 150, 150), 750, y_offset + 30});
}

