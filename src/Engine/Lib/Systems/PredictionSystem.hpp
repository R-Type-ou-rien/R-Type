#pragma once

#include <functional>
#include <iostream>
#include "../Components/PredictionComponent.hpp"
#include "../../Core/ECS/Registry/registry.hpp"
#include "../../Inputs/InputManager/ClientInputManager.hpp"

// Signature de la fonction physique : (Entity, Registry, Inputs, dt)
using PhysicsSimulationCallback = std::function<void(Entity, Registry&, const InputSnapshot&, float)>;

class PredictionSystem {
public:
    explicit PredictionSystem(PhysicsSimulationCallback logic) : _logic(logic) {}

    // 1. PRÉDICTION (À chaque frame client)
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

            pred.history.push_back({currentTick, currentInputs, transform});

            if (pred.history.size() > pred.maxHistorySize) {
                pred.history.pop_front();
            }
        }
    }

    // 2. RÉCONCILIATION (Quand on reçoit un packet serveur)
    void onServerUpdate(Registry& registry, Entity entity, const transform_component_s& serverState, uint32_t serverTick) {
        if (!registry.hasComponent<PredictionComponent>(entity)) return;

        auto& pred = registry.getComponent<PredictionComponent>(entity);
        auto& currentTransform = registry.getComponent<transform_component_s>(entity);

        // A. Supprimer l'historique obsolète (plus vieux que le serveur)
        while (!pred.history.empty() && pred.history.front().tick < serverTick) {
            pred.history.pop_front();
        }

        if (pred.history.empty()) {
            // Pas d'historique correspondant, on accepte juste la pos serveur
            currentTransform = serverState;
            return; 
        }

        // B. Trouver l'état qu'on AVAIT prédit pour ce tick
        auto& historyStep = pred.history.front();

        // C. Comparer (Seuil d'erreur)
        float dx = serverState.x - historyStep.state.x;
        float dy = serverState.y - historyStep.state.y;
        
        // Seuil de tolérance (ex: 1.0 pixel)
        if ((dx * dx + dy * dy) > 1.0f) {
            std::cout << "[LAG] Correction: " << dx << ", " << dy << " (Tick " << serverTick << ")" << std::endl;

            // 1. Appliquer la vérité serveur comme base
            currentTransform = serverState;

            // 2. REPLAY : Rejouer tous les inputs depuis le tick serveur jusqu'à maintenant
            // On saute le premier élément car c'est celui qui correspond au tick serveur (qu'on vient de corriger)
            for (size_t i = 1; i < pred.history.size(); ++i) {
                auto& step = pred.history[i];
                
                // On utilise un dt fixe pour le replay pour la stabilité (ex: 1/60)
                // Ou idéalement, on devrait stocker le dt dans SimulationStep
                float replayDt = 0.0166f; 

                _logic(entity, registry, step.inputs, replayDt);

                // Mettre à jour l'historique avec la nouvelle position corrigée
                step.state = currentTransform;
            }
        } else {
            // Si la prédiction était bonne, on ne fait rien ?
            // ATTENTION : Si le code réseau a écrasé la position actuelle avec la vieille position serveur juste avant d'appeler cette fonction,
            // il faut absolument restaurer la position prédite "actuelle".
            // Dans notre cas, on suppose que 'currentTransform' contient déjà la position "vieille" venant du packet.
            // Donc si la différence est minime, on DOIT remettre la position prédite la plus récente (celle du bout de l'historique)
            // pour éviter que le joueur ne "saute" en arrière visuellement.
            if (!pred.history.empty()) {
                currentTransform = pred.history.back().state;
            }
        }
    }

private:
    PhysicsSimulationCallback _logic;
};