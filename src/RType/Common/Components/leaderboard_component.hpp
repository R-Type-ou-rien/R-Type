#pragma once

#include <vector>
#include <string>

struct PlayerScoreEntry {
    Entity player_entity;
    std::string player_name;
    int score;
    bool is_alive;
};

struct LeaderboardComponent {
    static constexpr auto name = "LeaderboardComponent";
    std::vector<PlayerScoreEntry> entries;
    bool is_displayed = false;
    float display_start_time = 0.0f;
    bool ui_created = false;
    bool victory = false;
};
