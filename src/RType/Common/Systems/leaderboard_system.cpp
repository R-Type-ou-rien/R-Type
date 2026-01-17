#include "leaderboard_system.hpp"
#include "../Components/leaderboard_component.hpp"
#include "Components/StandardComponents.hpp"
#include "Components/NetworkComponents.hpp"
#include "../Systems/score.hpp"
#include "../Systems/health.hpp"
#include <algorithm>
#include <string>
#include <vector>
#include <utility>
#include <iostream>

class LeaderBoard {
   public:
    float start_y;
    float player_score_x;
    float line_height;
    unsigned int font_size;
    unsigned int title_font_size;
    unsigned int game_over_font_size;
    float title_x;
    float title_y;
    float game_over_x;
    float game_over_y;

    LeaderBoard(float width, float height) {
        float center_x = width / 2.0f;

        game_over_x = center_x - (width * 0.135f);
        game_over_y = height * 0.185f;
        title_x = center_x - (width * 0.11f);
        title_y = height * 0.35f;
        start_y = height * 0.50f;
        player_score_x = center_x - (width * 0.17f);
        line_height = height * 0.051f;
        font_size = static_cast<unsigned int>(height * 0.033f);
        title_font_size = static_cast<unsigned int>(height * 0.044f);
        game_over_font_size = static_cast<unsigned int>(height * 0.066f);
    }
};

static bool has_tag(const TagComponent& component, const std::string& target) {
    return std::find(component.tags.begin(), component.tags.end(), target) != component.tags.end();
}

template <typename Filter, typename NameGen>
static std::vector<PlayerScoreEntry> collect_scores(Registry& registry, Filter filter, NameGen name_gen) {
    std::vector<PlayerScoreEntry> scores;
    auto& entities = registry.getEntities<TagComponent>();

    for (auto entity : entities) {
        if (!registry.hasComponent<TagComponent>(entity))
            continue;

        const auto& tags = registry.getConstComponent<TagComponent>(entity);
        if (!filter(tags))
            continue;

        PlayerScoreEntry entry;
        entry.player_entity = entity;
        entry.player_name = name_gen(registry, entity, static_cast<int>(scores.size()) + 1);
        entry.score = 0;
        entry.is_alive = false;

        if (registry.hasComponent<ScoreComponent>(entity)) {
            entry.score = registry.getConstComponent<ScoreComponent>(entity).current_score;
        }
        if (registry.hasComponent<HealthComponent>(entity)) {
            entry.is_alive = registry.getConstComponent<HealthComponent>(entity).current_hp > 0;
        }

        scores.push_back(entry);
    }
    return scores;
}

void LeaderboardSystem::update(Registry& registry, system_context context) {
    auto& entities = registry.getEntities<LeaderboardComponent>();
    if (entities.empty())
        return;

    auto destroy_leaderboard_ui = [&registry]() {
        auto& tagged = registry.getEntities<TagComponent>();
        std::vector<Entity> to_destroy;
        to_destroy.reserve(tagged.size());

        for (auto e : tagged) {
            if (!registry.hasComponent<TagComponent>(e))
                continue;
            const auto& tags = registry.getConstComponent<TagComponent>(e);
            bool should_destroy = false;
            for (const auto& t : tags.tags) {
                if (t == "LEADERBOARD" || t == "VICTORY_TIMER") {
                    should_destroy = true;
                    break;
                }
            }
            if (should_destroy)
                to_destroy.push_back(e);
        }

        for (auto e : to_destroy) {
            registry.destroyEntity(e);
        }
    };

    float deltaTime = context.dt;

#if defined(CLIENT_BUILD)
    float width = static_cast<float>(context.window.getSize().x);
    float height = static_cast<float>(context.window.getSize().y);
#else
    float width = 1920.0f;
    float height = 1080.0f;
#endif
    LeaderBoard layout(width, height);

    for (auto entity : entities) {
        auto& leaderboard = registry.getComponent<LeaderboardComponent>(entity);

        if (!leaderboard.is_displayed) {
            destroy_leaderboard_ui();
            registry.destroyEntity(entity);
            continue;
        }

        leaderboard.elapsed_time += deltaTime;

        if (leaderboard.victory && leaderboard.elapsed_time >= leaderboard.auto_hide_duration) {
            destroy_leaderboard_ui();

            leaderboard.is_displayed = false;
            continue;
        }

        if (leaderboard.is_displayed && !leaderboard.ui_created) {
            leaderboard.ui_created = true;
            Entity gameOverTitle = registry.createEntity();
            Entity leaderboardTitle = registry.createEntity();

            {
                TagComponent tag;
                tag.tags.push_back("LEADERBOARD");
                registry.addComponent<TagComponent>(gameOverTitle, tag);
                registry.addComponent<TagComponent>(leaderboardTitle, tag);
            }

            if (leaderboard.victory) {
                registry.addComponent<TextComponent>(
                    gameOverTitle,
                    {"VICTORY", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf",
                     layout.game_over_font_size, sf::Color::Green, layout.game_over_x, layout.game_over_y});
            } else {
                registry.addComponent<TextComponent>(
                    gameOverTitle,
                    {"GAME OVER", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf",
                     layout.game_over_font_size, sf::Color::Red, layout.game_over_x, layout.game_over_y});
            }

            registry.addComponent<TextComponent>(
                leaderboardTitle, {"Ranking", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf",
                                   layout.title_font_size, sf::Color::Cyan, layout.title_x, layout.title_y});

            if (leaderboard.victory) {
                Entity timerEntity = registry.createEntity();
                TagComponent timerTag;
                timerTag.tags.push_back("LEADERBOARD");
                timerTag.tags.push_back("VICTORY_TIMER");
                registry.addComponent<TagComponent>(timerEntity, timerTag);

                int remaining = static_cast<int>(leaderboard.auto_hide_duration);
                std::string timerText = "Next Level: " + std::to_string(remaining) + "s";
                registry.addComponent<TextComponent>(
                    timerEntity, {timerText, "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 48,
                                  sf::Color::Yellow, 1400.0f, 900.0f});
            } else {
                // Return to Lobby Button
                Entity returnButton = registry.createEntity();
                TagComponent btnTag;
                btnTag.tags.push_back("LEADERBOARD");
                btnTag.tags.push_back("RETURN_BUTTON");
                registry.addComponent<TagComponent>(returnButton, btnTag);

                registry.addComponent<TextComponent>(
                    returnButton,
                    {"[ RETURN TO LOBBY ]", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 48,
                     sf::Color::White, layout.game_over_x + 50, layout.game_over_y + 600});
            }

            float y_offset = layout.start_y;

            for (const auto& entry : leaderboard.entries) {
                Entity scoreEntity = registry.createEntity();
                {
                    TagComponent tag;
                    tag.tags.push_back("LEADERBOARD");
                    registry.addComponent<TagComponent>(scoreEntity, tag);
                }
                std::string name_text = entry.player_name;
                std::string score_text = std::to_string(entry.score) + " pts";
                registry.addComponent<TextComponent>(
                    scoreEntity,
                    {name_text + ": " + score_text, "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf",
                     layout.font_size, sf::Color::White, layout.player_score_x, y_offset});
                y_offset += layout.line_height;
            }
        } else if (leaderboard.victory && leaderboard.ui_created) {
            auto& timer_entities = registry.getEntities<TagComponent>();
            for (auto timer_entity : timer_entities) {
                if (!registry.hasComponent<TagComponent>(timer_entity))
                    continue;

                auto& tags = registry.getConstComponent<TagComponent>(timer_entity);
                bool is_timer = false;
                for (const auto& tag : tags.tags) {
                    if (tag == "VICTORY_TIMER") {
                        is_timer = true;
                        break;
                    }
                }
                if (is_timer && registry.hasComponent<TextComponent>(timer_entity)) {
                    int remaining = static_cast<int>(leaderboard.auto_hide_duration - leaderboard.elapsed_time);
                    if (remaining < 0)
                        remaining = 0;
                    std::string timerText = "Next Level: " + std::to_string(remaining) + "s";
                    registry.getComponent<TextComponent>(timer_entity).text = timerText;

                    static float last_log_time = 0;
                    if (leaderboard.elapsed_time - last_log_time >= 1.0f) {
                        last_log_time = leaderboard.elapsed_time;
                    }
                }
            }
        }
    }
}

Entity LeaderboardSystem::createLeaderboard(Registry& registry, bool victory) {
    auto scores = collect_scores(
        registry, [](const TagComponent& t) { return has_tag(t, "LEADERBOARD_DATA"); },
        [](Registry& r, Entity e, int idx) {
            if (r.hasComponent<NetworkIdentity>(e)) {
                return "Player " + std::to_string(r.getConstComponent<NetworkIdentity>(e).ownerId);
            }
            return "Player " + std::to_string(idx);
        });

    // Local get scores
    if (scores.empty()) {
        scores = collect_scores(
            registry, [](const TagComponent& t) { return has_tag(t, "PLAYER"); },
            [](Registry&, Entity, int idx) { return "Player " + std::to_string(idx); });
    }

    std::sort(scores.begin(), scores.end(),
              [](const PlayerScoreEntry& a, const PlayerScoreEntry& b) { return a.score > b.score; });

    Entity leaderboardEntity = registry.createEntity();
    LeaderboardComponent leaderboard;
    leaderboard.entries = std::move(scores);
    leaderboard.is_displayed = true;
    leaderboard.victory = victory;
    registry.addComponent<LeaderboardComponent>(leaderboardEntity, leaderboard);
    return leaderboardEntity;
}
