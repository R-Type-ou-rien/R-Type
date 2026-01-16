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
    float elapsed_time = 0.0f;  // Track how long leaderboard has been shown
    bool ui_created = false;
    bool victory = false;
    float auto_hide_duration = 10.0f;  // Auto-hide after 10 seconds for victory
};
