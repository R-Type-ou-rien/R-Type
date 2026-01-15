#include "leaderboard_system.hpp"
#include "../Components/leaderboard_component.hpp"
#include "Components/StandardComponents.hpp"
#include "Components/NetworkComponents.hpp"
#include "../Systems/score.hpp"
#include "../Systems/health.hpp"
#include <algorithm>
#include <string>
#include <vector>

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

template<typename Filter, typename NameGen>
static std::vector<PlayerScoreEntry> collect_scores(Registry& registry, Filter filter, NameGen name_gen) {
    std::vector<PlayerScoreEntry> scores;
    auto& entities = registry.getEntities<TagComponent>();
    
    for (auto entity : entities) {
        if (!registry.hasComponent<TagComponent>(entity)) continue;
        
        const auto& tags = registry.getConstComponent<TagComponent>(entity);
        if (!filter(tags)) continue;

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

    if (leaderboard.is_displayed && !leaderboard.ui_created) {
      leaderboard.ui_created = true;
      Entity gameOverTitle = registry.createEntity();
      Entity leaderboardTitle = registry.createEntity();

      if (leaderboard.victory) {
        registry.addComponent<TextComponent>(gameOverTitle, {"VICTORY", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", layout.game_over_font_size, sf::Color::Green, layout.game_over_x, layout.game_over_y});
      } else {
        registry.addComponent<TextComponent>(gameOverTitle, {"GAME OVER", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", layout.game_over_font_size, sf::Color::Red, layout.game_over_x, layout.game_over_y});
      }

      registry.addComponent<TextComponent>(leaderboardTitle, {"Ranking", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", layout.title_font_size, sf::Color::Cyan, layout.title_x, layout.title_y});

      float y_offset = layout.start_y;

      for (const auto& entry : leaderboard.entries) {
        Entity scoreEntity = registry.createEntity();                
        std::string name_text = entry.player_name;
        std::string score_text = std::to_string(entry.score) + " pts";
        registry.addComponent<TextComponent>(scoreEntity, {name_text + ": " + score_text, "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", layout.font_size, sf::Color::White, layout.player_score_x, y_offset});
        y_offset += layout.line_height;
      }
    }
  }
}

Entity LeaderboardSystem::createLeaderboard(Registry& registry, bool victory) {
    auto scores = collect_scores(registry, 
        [](const TagComponent& t) { return has_tag(t, "LEADERBOARD_DATA"); },
        [](Registry& r, Entity e, int idx) {
            if (r.hasComponent<NetworkIdentity>(e)) {
                return "Player " + std::to_string(r.getConstComponent<NetworkIdentity>(e).ownerId);
            }
            return "Player " + std::to_string(idx);
        }
    );

    // Local get scores
    if (scores.empty()) {
        scores = collect_scores(registry,
            [](const TagComponent& t) { return has_tag(t, "PLAYER"); },
            [](Registry&, Entity, int idx) {
                return "Player " + std::to_string(idx);
            }
        );
    }

    std::sort(scores.begin(), scores.end(), 
              [](const PlayerScoreEntry& a, const PlayerScoreEntry& b) {
                  return a.score > b.score;
              });

    Entity leaderboardEntity = registry.createEntity();
    LeaderboardComponent leaderboard;
    leaderboard.entries = std::move(scores);
    leaderboard.is_displayed = true;
    leaderboard.victory = victory;
    registry.addComponent<LeaderboardComponent>(leaderboardEntity, leaderboard);
    return leaderboardEntity;
}
