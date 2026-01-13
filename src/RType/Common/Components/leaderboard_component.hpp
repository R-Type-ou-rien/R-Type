#pragma once

#include <vector>
#include <string>
#include "ECS.hpp"

struct PlayerScoreEntry {
    Entity player_entity;
    std::string player_name;
    int score;
    bool is_alive;
};

// Composant pour le leaderboard
struct LeaderboardComponent {
    static constexpr auto name = "LeaderboardComponent";
    std::vector<PlayerScoreEntry> entries;
    bool is_displayed = false;
    float display_start_time = 0.0f;
};
