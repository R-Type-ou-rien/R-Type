#pragma once

#include "ISystem.hpp"
#include "registry.hpp"

struct ScoreComponent {
    static constexpr auto name = "ScoreComponent";
    int current_score = 0;
    int high_score = 0;
};

struct ScoreValueComponent {
    static constexpr auto name = "ScoreValueComponent";
    int value = 100;
};

class ScoreSystem : public ISystem {
public:
    ScoreSystem() = default;
    ~ScoreSystem() = default;
    void update(Registry& registry, system_context context) override;
    
    static void addScore(Registry& registry, int points);
    static int getScore(Registry& registry);
};
