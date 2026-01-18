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
#include <vector>

void GameStateSystem::update(Registry& registry, system_context context) {
#if defined(SERVER_BUILD)
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

    auto& lobbies = context.lobby_manager->getAllLobbies();
    for (auto const& [lobbyId, lobby] : lobbies) {
        if (lobby.getState() != engine::core::Lobby::State::IN_GAME) {
            continue;
        }

        any_in_game_lobby = true;

        for (const auto& client : lobby.getClients()) {
            PlayerInfo player;
            player.client_id = client.id;
            player.entity_id = -1;
            player.hp = 0;
            player.score = 0;
            player.found_in_registry = false;

            auto& entities_with_network_id = registry.getEntities<NetworkIdentity>();
            for (auto entity : entities_with_network_id) {
                auto& net_id = registry.getConstComponent<NetworkIdentity>(entity);
                if (net_id.ownerId == client.id) {
                    player.entity_id = entity;
                    player.found_in_registry = true;

                    if (registry.hasComponent<HealthComponent>(entity)) {
                        auto& health = registry.getConstComponent<HealthComponent>(entity);
                        player.hp = health.current_hp;
                        if (player.hp > 0)
                            alive_count++;
                    }

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

    if (all_players.empty()) {
        if (!any_in_game_lobby) {
            _gameOverSent = false;
            _victorySent = false;
        }
        return;
    }

    if (!any_in_game_lobby) {
        _gameOverSent = false;
        _victorySent = false;
        return;
    }

    if (alive_count == 0) {
        if (!_gameOverSent) {
            auto network_instance = context.network.getNetworkInstance();
            if (std::holds_alternative<std::shared_ptr<network::Server>>(network_instance)) {
                auto server = std::get<std::shared_ptr<network::Server>>(network_instance);

                // network::GameOverPacket gameOverPacket;
                // gameOverPacket.victory = false;
                // gameOverPacket.player_count = 0;

                // for (const auto& player : all_players) {
                //     if (gameOverPacket.player_count >= 8)
                //         break;

                //     network::PlayerScore playerScore;
                //     playerScore.client_id = player.client_id;
                //     playerScore.score = player.score;
                //     playerScore.is_alive = (player.hp > 0);

                //     gameOverPacket.players[gameOverPacket.player_count] = playerScore;
                //     gameOverPacket.player_count++;
                // }

                for (auto const& [lobbyId, lobby] : lobbies) {
                    if (lobby.getState() != engine::core::Lobby::State::IN_GAME) {
                        continue;
                    }

                    // for (const auto& client : lobby.getClients()) {
                    //     server->AddMessageToPlayer(network::GameEvents::S_GAME_OVER, client.id, gameOverPacket);
                    // }
                }

                _gameOverSent = true;
            }
        }

        return;
    }

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
                    break;
                }
            }
            if (boss_exists)
                break;
        }
    }

    if (boss_spawned && !boss_exists && !_victorySent) {
        auto network_instance = context.network.getNetworkInstance();
        if (std::holds_alternative<std::shared_ptr<network::Server>>(network_instance)) {
            auto server = std::get<std::shared_ptr<network::Server>>(network_instance);

            // network::GameOverPacket gameOverPacket;
            // gameOverPacket.victory = true;
            // gameOverPacket.player_count = 0;

            // for (const auto& player : all_players) {
            //     if (gameOverPacket.player_count >= 8)
            //         break;

            //     network::PlayerScore playerScore;
            //     playerScore.client_id = player.client_id;
            //     playerScore.score = player.score;
            //     playerScore.is_alive = (player.hp > 0);
            //     gameOverPacket.players[gameOverPacket.player_count] = playerScore;
            //     gameOverPacket.player_count++;
            // }

            for (auto const& [lobbyId, lobby] : lobbies) {
                if (lobby.getState() != engine::core::Lobby::State::IN_GAME) {
                    continue;
                }

                // for (const auto& client : lobby.getClients()) {
                //     server->AddMessageToPlayer(network::GameEvents::S_GAME_OVER, client.id, gameOverPacket);
                // }
            }

            _victorySent = true;
        }
    }
#endif
}
