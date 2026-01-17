/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Level Transition System - Animations entre les niveaux
*/

#pragma once

#include "ISystem.hpp"
#include "registry.hpp"
#include "src/RType/Common/Components/spectator_component.hpp"
#include "Components/StandardComponents.hpp"

class LevelTransitionSystem : public ISystem {
   public:
    LevelTransitionSystem() = default;
    ~LevelTransitionSystem() = default;

    void update(Registry& registry, system_context context) override;
    
   private:
    enum State {
        WAITING_BOSS,
        LEADERBOARD,
        TRANSITIONING
    };

    State _state = WAITING_BOSS;
    float _timer = 0.0f;
    float _leaderboard_duration = 10.0f; // Default, will be loaded from config
    std::string _next_level = ""; // Default/Fallback

    void loadGameConfig();
};
