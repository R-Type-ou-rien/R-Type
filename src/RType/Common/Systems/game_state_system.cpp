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
#include <iostream>
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
    
    // Parcourir TOUS les lobbies en jeu pour trouver TOUS les joueurs
    auto& lobbies = context.lobby_manager->getAllLobbies();
    for (auto const& [lobbyId, lobby] : lobbies) {
        if (lobby.getState() != engine::core::Lobby::State::IN_GAME) {
            continue;
        }
        
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
            std::cout << "[GameStateSystem] Client " << player.client_id 
                      << " | Entity " << player.entity_id 
                      << " | HP: " << player.hp 
                      << " | Score: " << player.score 
                      << " | " << (player.found_in_registry ? "✅ IN REGISTRY" : "❌ DESTROYED")
                      << std::endl;
        }
    }
    
    std::cout << "[GameStateSystem] === SUMMARY: Total=" << all_players.size() 
              << " | Alive=" << alive_count << " ===" << std::endl;
    
    // Si aucun joueur ou tous vivants, rien à faire
    if (all_players.empty() || alive_count == all_players.size()) {
        return;
    }
    
    // SERVEUR SEULEMENT : Envoyer S_GAME_OVER quand tous les joueurs sont morts
    if (alive_count == 0 && !_gameOverSent) {
        std::cout << "🌐 [GameStateSystem] All players dead! Sending S_GAME_OVER..." << std::endl;
        
        auto network_instance = context.network.getNetworkInstance();
        if (std::holds_alternative<std::shared_ptr<network::Server>>(network_instance)) {
            auto server = std::get<std::shared_ptr<network::Server>>(network_instance);
            
            // Créer le packet avec les scores de TOUS les joueurs
            network::GameOverPacket gameOverPacket;
            gameOverPacket.victory = false;
            gameOverPacket.player_count = 0;
            
            std::cout << "  📊 Collecting scores from " << all_players.size() << " players..." << std::endl;
            
            for (const auto& player : all_players) {
                if (gameOverPacket.player_count >= 8) break;  // Max 8 joueurs
                
                network::PlayerScore playerScore;
                playerScore.client_id = player.client_id;
                playerScore.score = player.score;
                playerScore.is_alive = (player.hp > 0);
                
                gameOverPacket.players[gameOverPacket.player_count] = playerScore;
                gameOverPacket.player_count++;
                
                std::cout << "    ✅ Player " << player.client_id 
                          << " | Score: " << player.score << " pts"
                          << " | " << (playerScore.is_alive ? "ALIVE" : "DEAD") << std::endl;
            }
            
            std::cout << "  📨 Broadcasting to all clients..." << std::endl;
            
            // Envoyer à TOUS les clients dans les lobbies en jeu
            for (auto const& [lobbyId, lobby] : lobbies) {
                if (lobby.getState() != engine::core::Lobby::State::IN_GAME) {
                    continue;
                }
                
                for (const auto& client : lobby.getClients()) {
                    server->AddMessageToPlayer(network::GameEvents::S_GAME_OVER, client.id, gameOverPacket);
                    std::cout << "    📨 Sent S_GAME_OVER with " << gameOverPacket.player_count 
                              << " players to client " << client.id << std::endl;
                }
            }
            
            _gameOverSent = true;
        }
    }
#endif
}
