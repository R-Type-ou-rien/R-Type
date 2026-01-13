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

    if (env.isServer() || _gameOver || _victory) {
        return;
    }

    Entity player_id = (_player) ? _player->getId() : findPlayerEntity(ecs.registry);

    // Update Status Display
    if (player_id != -1 && _statusDisplayEntity != static_cast<Entity>(-1)) {
        if (ecs.registry.hasComponent<StatusDisplayComponent>(_statusDisplayEntity)) {
            ecs.registry.getComponent<StatusDisplayComponent>(_statusDisplayEntity).player_entity = player_id;
        }
    }

    // Update Timer
    if (_timerEntity != static_cast<Entity>(-1) && ecs.registry.hasComponent<TextComponent>(_timerEntity)) {
        auto& game_timers = ecs.registry.getEntities<GameTimerComponent>();
        if (!game_timers.empty()) {
            auto& timer = ecs.registry.getComponent<GameTimerComponent>(game_timers[0]);
            ecs.registry.getComponent<TextComponent>(_timerEntity).text = 
                "Time: " + std::to_string(static_cast<int>(timer.elapsed_time)) + "s";
        }
    }

    // Update Boss HP
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
                    
                    // Couleur dynamique
                    if (percent > 50) boss_hp_text.color = sf::Color::Red;
                    else if (percent > 25) boss_hp_text.color = sf::Color(255, 165, 0); // Orange
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

    // CLIENT SEULEMENT : Vérifier si on a reçu S_GAME_OVER du serveur
    if (!env.isServer()) {
        auto& gameOverEntities = ecs.registry.getEntities<GameOverNotification>();
        if (!gameOverEntities.empty()) {
            auto& notification = ecs.registry.getConstComponent<GameOverNotification>(gameOverEntities[0]);
            _gameOver = !notification.victory;
            _victory = notification.victory;
            
            // Supprimer le composant pour ne pas le traiter plusieurs fois
            ecs.registry.destroyEntity(gameOverEntities[0]);
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
    if (env.isServer()) return;

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

    // Titre GAME OVER ou VICTORY
    Entity gameOverTitle = ecs.registry.createEntity();
    std::string game_status = victory ? "VICTORY!" : "GAME OVER";
    sf::Color game_status_color = victory ? sf::Color::Green : sf::Color::Red;
    ecs.registry.addComponent<TextComponent>(
        gameOverTitle, {game_status, "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 72,
                game_status_color, 700, 200});

    // Sous-titre du leaderboard
    Entity subtitle = ecs.registry.createEntity();
    std::string subtitle_text = victory ? "Congratulations!" : "Better luck next time!";
    ecs.registry.addComponent<TextComponent>(
        subtitle, {subtitle_text, "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 32,
                sf::Color::White, 750, 300});
    
    // Titre LEADERBOARD
    Entity leaderboardTitle = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        leaderboardTitle, {"LEADERBOARD", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 48,
                sf::Color::Cyan, 750, 380});
    
    // Afficher le score total combiné avec un style plus visible
    float y_offset = 460;
    if (total_combined_score > 0) {
        Entity totalScoreEntity = ecs.registry.createEntity();
        std::string total_text = "Total Score: " + std::to_string(total_combined_score) + " pts";
        ecs.registry.addComponent<TextComponent>(
            totalScoreEntity, {total_text, "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 36,
                            sf::Color(255, 255, 100), 650, y_offset});
        y_offset += 70;
    } else {
        y_offset += 20;
    }
    
    // Afficher chaque entrée avec un style amélioré
    int rank = 1;
    for (const auto& entry : player_scores) {
        Entity scoreEntity = ecs.registry.createEntity();
        
        std::string rank_icon;
        if (rank == 1 && player_scores.size() > 1) {
            rank_icon = "1st ";
        } else if (rank == 2) {
            rank_icon = "2nd ";
        } else if (rank == 3) {
            rank_icon = "3rd ";
        } else {
            rank_icon = "#" + std::to_string(rank) + " ";
        }
        
        std::string name_text = entry.player_name;
        std::string score_text = std::to_string(entry.score) + " pts";
        std::string status_text = entry.is_alive ? " ✓" : " ✗";
        
        std::string full_text = rank_icon + name_text + ": " + score_text + status_text;
        
        sf::Color entry_color = sf::Color::White;
        if (rank == 1 && player_scores.size() > 1) {
            entry_color = sf::Color(255, 215, 0); // Or brillant
        } else if (rank == 2) {
            entry_color = sf::Color(192, 192, 192); // Argent
        } else if (rank == 3) {
            entry_color = sf::Color(205, 127, 50); // Bronze
        } else {
            entry_color = sf::Color(200, 200, 255); // Bleu clair pour les autres
        }
        
        ecs.registry.addComponent<TextComponent>(
            scoreEntity, {full_text, "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 36,
                          entry_color, 600, y_offset});
        
        y_offset += 55;
        rank++;
    }

    // Message de fin avec style amélioré
    Entity finalMsg = ecs.registry.createEntity();
    std::string final_text = victory ? "Congratulations! Press ESC to quit" : "Try again! Press ESC to quit";
    ecs.registry.addComponent<TextComponent>(
        finalMsg, {final_text, "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 28,
                   sf::Color(180, 180, 180), 580, y_offset + 40});
}

