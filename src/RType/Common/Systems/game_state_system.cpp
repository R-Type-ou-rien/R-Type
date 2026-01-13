/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Game State System Implementation
*/

#include "game_state_system.hpp"
#include "Network.hpp"
#include "src/Engine/Core/NetworkEngine/NetworkEngine.hpp"
#include "src/Engine/Core/LobbyManager.hpp"
#include "src/RType/Common/Systems/score.hpp"
#include <variant>

void GameStateSystem::update(Registry& registry, system_context context) {
#if defined(SERVER_BUILD)
    // SOLUTION: Utiliser le LobbyManager pour obtenir la liste des clients
    // au lieu de chercher dans le registre ECS (les joueurs morts sont supprimés!)
    if (!context.lobby_manager) {
        return;
    }
    
    struct PlayerInfo {
        uint32_t client_id;
        Entity entity_id;
        int hp;
        int score;
        bool found_in_registry;
    };
    
    std::vector<PlayerInfo> all_players;
    int alive_count = 0;
    bool any_in_game_lobby = false;
    
    // Parcourir TOUS les lobbies en jeu pour trouver TOUS les joueurs
    auto& lobbies = context.lobby_manager->getAllLobbies();
    for (auto const& [lobbyId, lobby] : lobbies) {
        if (lobby.getState() != engine::core::Lobby::State::IN_GAME) {
            continue;
        }

        any_in_game_lobby = true;
        
        // Pour chaque client dans le lobby
        for (const auto& client : lobby.getClients()) {
            PlayerInfo player;
            player.client_id = client.id;
            player.entity_id = -1;
            player.hp = 0;
            player.score = 0;
            player.found_in_registry = false;
            
            // Trouver l'entité du joueur via NetworkIdentity
            auto& entities_with_network_id = registry.getEntities<NetworkIdentity>();
            for (auto entity : entities_with_network_id) {
                auto& net_id = registry.getConstComponent<NetworkIdentity>(entity);
                if (net_id.ownerId == client.id) {
                    player.entity_id = entity;
                    player.found_in_registry = true;
                    
                    // Récupérer HP
                    if (registry.hasComponent<HealthComponent>(entity)) {
                        auto& health = registry.getConstComponent<HealthComponent>(entity);
                        player.hp = health.current_hp;
                        if (player.hp > 0) alive_count++;
                    }
                    
                    // Récupérer score
                    if (registry.hasComponent<ScoreComponent>(entity)) {
                        auto& score_comp = registry.getConstComponent<ScoreComponent>(entity);
                        player.score = score_comp.current_score;
                    }
                    break;
                }
            }
            
            all_players.push_back(player);
        }
    }
    
    // Si aucun joueur, rien à faire
    if (all_players.empty()) {
        // If no lobby is currently in-game, we're between matches: reset one-shot flags
        if (!any_in_game_lobby) {
            _gameOverSent = false;
            _victorySent = false;
        }
        return;
    }

    // If we're not in an active match, reset flags and stop
    if (!any_in_game_lobby) {
        _gameOverSent = false;
        _victorySent = false;
        return;
    }

    // SERVEUR SEULEMENT : Envoyer S_GAME_OVER quand tous les joueurs sont morts (prioritaire sur la victoire)
    if (alive_count == 0) {
        if (!_gameOverSent) {
            auto network_instance = context.network.getNetworkInstance();
            if (std::holds_alternative<std::shared_ptr<network::Server>>(network_instance)) {
                auto server = std::get<std::shared_ptr<network::Server>>(network_instance);

                // Créer le packet avec les scores de TOUS les joueurs
                network::GameOverPacket gameOverPacket;
                gameOverPacket.victory = 0;  // 0 = défaite
                gameOverPacket.player_count = 0;

                for (const auto& player : all_players) {
                    if (gameOverPacket.player_count >= 8)
                        break;

                    network::PlayerScore playerScore;
                    playerScore.client_id = player.client_id;
                    playerScore.score = player.score;
                    playerScore.is_alive = (player.hp > 0) ? 1 : 0;

                    gameOverPacket.players[gameOverPacket.player_count] = playerScore;
                    gameOverPacket.player_count++;
                }

                // Envoyer à TOUS les clients dans les lobbies en jeu
                for (auto const& [lobbyId, lobby] : lobbies) {
                    if (lobby.getState() != engine::core::Lobby::State::IN_GAME) {
                        continue;
                    }

                    for (const auto& client : lobby.getClients()) {
                        std::cout << "[GAMESTATE DEBUG] Sending S_GAME_OVER to client " << client.id 
                                  << " victory=" << (int)gameOverPacket.victory 
                                  << " player_count=" << gameOverPacket.player_count << std::endl;
                        server->AddMessageToPlayer(network::GameEvents::S_GAME_OVER, client.id, gameOverPacket);
                    }
                }

                _gameOverSent = true;
            }
        }

        // Even if the boss is dead too, defeat must win.
        return;
    }

    // Détection victoire: boss vaincu (uniquement s'il reste au moins 1 joueur vivant)
    bool boss_spawned = false;
    {
        auto& spawners = registry.getEntities<EnemySpawnComponent>();
        for (auto spawner : spawners) {
            if (!registry.hasComponent<EnemySpawnComponent>(spawner))
                continue;
            auto& spawn_comp = registry.getConstComponent<EnemySpawnComponent>(spawner);
            if (spawn_comp.boss_arrived) {
                boss_spawned = true;
                break;
            }
        }
    }
    
    std::cout << "[GAMESTATE DEBUG] boss_spawned=" << boss_spawned << " alive_count=" << alive_count << std::endl;

    bool boss_exists = false;
    if (boss_spawned) {
        auto& tagged = registry.getEntities<TagComponent>();
        for (auto entity : tagged) {
            if (!registry.hasComponent<TagComponent>(entity))
                continue;
            auto& tags = registry.getConstComponent<TagComponent>(entity);
            for (const auto& tag : tags.tags) {
                if (tag == "BOSS") {
                    boss_exists = true;
                    std::cout << "[GAMESTATE DEBUG] Found BOSS entity=" << entity << std::endl;
                    break;
                }
            }
            if (boss_exists)
                break;
        }
    }
    
    std::cout << "[GAMESTATE DEBUG] boss_exists=" << boss_exists << " _victorySent=" << _victorySent << std::endl;

    if (boss_spawned && !boss_exists && !_victorySent) {
        std::cout << "[GAMESTATE DEBUG] VICTORY DETECTED! Boss spawned and destroyed. Sending S_GAME_OVER(victory=true)" << std::endl;
        auto network_instance = context.network.getNetworkInstance();
        if (std::holds_alternative<std::shared_ptr<network::Server>>(network_instance)) {
            auto server = std::get<std::shared_ptr<network::Server>>(network_instance);

            network::GameOverPacket gameOverPacket;
            gameOverPacket.victory = 1;  // 1 = victoire
            gameOverPacket.player_count = 0;

            for (const auto& player : all_players) {
                if (gameOverPacket.player_count >= 8)
                    break;

                network::PlayerScore playerScore;
                playerScore.client_id = player.client_id;
                playerScore.score = player.score;
                playerScore.is_alive = (player.hp > 0) ? 1 : 0;
                gameOverPacket.players[gameOverPacket.player_count] = playerScore;
                gameOverPacket.player_count++;
            }

            // Envoyer à TOUS les clients dans les lobbies en jeu
            for (auto const& [lobbyId, lobby] : lobbies) {
                if (lobby.getState() != engine::core::Lobby::State::IN_GAME) {
                    continue;
                }

                for (const auto& client : lobby.getClients()) {
                    server->AddMessageToPlayer(network::GameEvents::S_GAME_OVER, client.id, gameOverPacket);
                }
            }

            _victorySent = true;
        }
    }
#endif
}
