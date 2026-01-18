#pragma once

#include <deque>
#include <vector>
#include <map>
#include "StandardComponents.hpp"
#include "../../Inputs/InputAction.hpp"  // Pour l'enum Action

struct InputSnapshot {
    std::map<Action, bool> actions;

    bool isPressed(Action a) const {
        auto it = actions.find(a);
        return it != actions.end() && it->second;
    }
};

struct SimulationStep {
    uint32_t tick;
    InputSnapshot inputs;
    transform_component_s state;
    float dt;
};

struct PredictionComponent {
    static constexpr auto name = "PredictionComponent";

    std::deque<SimulationStep> history;
    size_t maxHistorySize = 120;
};