/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Level Transition System Implementation
*/

#include "level_transition.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include "../Components/spawn.hpp"
#include "../Components/game_over_notification.hpp"
#include "../Components/leaderboard_component.hpp"
#include "src/Engine/Core/Scene/SceneManager.hpp"
#include "src/Engine/Core/Scene/LevelConfig.hpp"
#include "src/Engine/Core/Scene/SceneLoader.hpp"

// We no longer need LeaderboardUI struct as we depend on LeaderboardComponent entities

void LevelTransitionSystem::update(Registry& registry, system_context context) {
    // Check if Leaderboard is active
    auto& leaderboards = registry.getEntities<LeaderboardComponent>();
    bool leaderboard_active = false;
    
    for (auto entity : leaderboards) {
        if (registry.hasComponent<LeaderboardComponent>(entity)) {
            leaderboard_active = true;
            break;
        }
    }
    
    if (leaderboard_active) {
        if (_state != LEADERBOARD) {
            _state = LEADERBOARD;
            _timer = 0.0f;
            loadGameConfig(); // Reload duration from config
            std::cout << "[LevelTransition] Leaderboard detected. Starting timer for " << _leaderboard_duration << "s." << std::endl;
        }
        
        float safe_dt = (context.dt > 0.1f) ? 0.1f : context.dt;
        _timer += safe_dt;
        
        static float last_print = 0.0f;
        if (_timer - last_print >= 1.0f) {
            std::cout << "[LevelTransition] Timer: " << _timer << "/" << _leaderboard_duration << std::endl;
            last_print = _timer;
        }
        
        if (_timer >= _leaderboard_duration) {
            std::cout << "[LevelTransition] Leaderboard duration expired. Destroying leaderboard to trigger next level." << std::endl;
            
            // Destroy all LeaderboardComponent entities
            // This signals GameManager to proceed to next level (via loadNextLevel)
            std::vector<Entity> to_destroy;
            for (auto entity : leaderboards) {
                if (registry.hasComponent<LeaderboardComponent>(entity)) {
                    to_destroy.push_back(entity);
                }
            }
            
            for (auto e : to_destroy) {
                registry.destroyEntity(e);
            }
            
            _state = WAITING_BOSS; // Reset state
        }
    } else {
        // No leaderboard active
        _state = WAITING_BOSS;
    }
}

void LevelTransitionSystem::loadGameConfig() {
    std::ifstream file("src/RType/Common/content/config/game.cfg");
    if (!file.is_open()) {
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("leaderboard_duration") != std::string::npos) {
            size_t pos = line.find("=");
            if (pos != std::string::npos) {
                try {
                    _leaderboard_duration = std::stof(line.substr(pos + 1));
                } catch (...) {}
            }
        }
    }
}

// showLeaderboard and hideLeaderboard are removed as GameManager/LeaderboardSystem handles UI creation
