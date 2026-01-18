#pragma once

#include <string>

struct LevelTransitionComponent {
    static constexpr auto name = "LevelTransitionComponent";
    enum class TransitionState { IDLE, FADE_OUT, LOADING, FADE_IN, FINISHED };

    TransitionState state = TransitionState::IDLE;
    std::string next_level_name;
    float timer = 0.0f;
    float duration = 1.0f;
};
