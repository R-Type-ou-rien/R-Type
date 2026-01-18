#pragma once

#include <functional>
#include <iostream>
#include "../Components/PredictionComponent.hpp"
#include "../../Core/ECS/Registry/registry.hpp"
#include "../../Inputs/InputManager/ClientInputManager.hpp"

using PhysicsSimulationCallback = std::function<void(Entity, Registry&, const InputSnapshot&, float)>;

class PredictionSystem {
   public:
    explicit PredictionSystem(PhysicsSimulationCallback logic) : _logic(logic) {}

    void updatePrediction(Registry& registry, ClientInputManager& inputManager, uint32_t currentTick, float dt) {
        if (!_logic)
            return;

        auto entities = registry.getEntities<PredictionComponent>();

        for (auto entity : entities) {
            if (!registry.hasComponent<transform_component_s>(entity))
                continue;

            auto& pred = registry.getComponent<PredictionComponent>(entity);
            auto& transform = registry.getComponent<transform_component_s>(entity);

            InputSnapshot currentInputs = inputManager.getCurrentInputSnapshot();

            _logic(entity, registry, currentInputs, dt);

            pred.history.push_back({currentTick, currentInputs, transform, dt});

            if (pred.history.size() > pred.maxHistorySize) {
                pred.history.pop_front();
            }
        }
    }

    void onServerUpdate(Registry& registry, Entity entity, const transform_component_s& serverState,
                        uint32_t serverTick) {
        if (!registry.hasComponent<PredictionComponent>(entity))
            return;

        auto& pred = registry.getComponent<PredictionComponent>(entity);
        auto& currentTransform = registry.getComponent<transform_component_s>(entity);

        while (!pred.history.empty() && pred.history.front().tick < serverTick) {
            pred.history.pop_front();
        }

        if (pred.history.empty()) {
            currentTransform = serverState;
            return;
        }

        auto& historyStep = pred.history.front();

        float dx = serverState.x - historyStep.state.x;
        float dy = serverState.y - historyStep.state.y;

        if ((dx * dx + dy * dy) > 1.0f) {
            currentTransform = serverState;
        } else {
            if (!pred.history.empty()) {
                currentTransform = pred.history.back().state;
            }
        }
    }

   private:
    PhysicsSimulationCallback _logic;
};