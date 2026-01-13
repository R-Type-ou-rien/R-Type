#pragma once

#include <deque>
#include <vector>
#include <map>
#include "StandardComponents.hpp"
#include "../../Inputs/InputAction.hpp" // Pour l'enum Action

// Structure légère pour stocker quels boutons étaient appuyés à un tick donné
struct InputSnapshot {
    std::map<Action, bool> actions;

    bool isPressed(Action a) const {
        auto it = actions.find(a);
        return it != actions.end() && it->second;
    }
};

// Une étape de l'historique de simulation
struct SimulationStep {
    uint32_t tick;
    InputSnapshot inputs;
    transform_component_s state;
    // On pourrait ajouter Velocity2D si la physique est complexe
};

// Le composant à ajouter au joueur local
struct PredictionComponent {
    static constexpr auto name = "PredictionComponent";
    
    std::deque<SimulationStep> history;
    size_t maxHistorySize = 120; // Env. 2 secondes à 60Hz
};