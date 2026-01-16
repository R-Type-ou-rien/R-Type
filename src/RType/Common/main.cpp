#include <iostream>
#include <ostream>
#include <type_traits>

#include "ClientGameEngine.hpp"
#include "GameEngineConfig.hpp"
#include "NetworkEngine/NetworkEngine.hpp"
#include "Components/StandardComponents.hpp"
#include "Lib/GameManager/GameManager.hpp"

template <typename T>
void setupPrediction(T& engine, GameManager& gm) {
    if constexpr (std::is_same_v<T, ClientGameEngine>) {
        engine.setPredictionLogic([&gm](Entity e, Registry& r, const InputSnapshot& inputs, float dt) {
            gm.predictionLogic(e, r, inputs, dt);
        });
    }
}

void registerComponents(GameEngine& engine)
{
    engine.registerNetworkComponent<DamageOnCollision>();
    engine.registerNetworkComponent<::GameTimerComponent>();
    engine.registerNetworkComponent<ChargedShotComponent>();
    engine.registerNetworkComponent<ShooterComponent>();
    engine.registerNetworkComponent<PatternComponent>();
    engine.registerNetworkComponent<ProjectileComponent>();
    engine.registerNetworkComponent<TeamComponent>();
    engine.registerNetworkComponent<PodComponent>();
    engine.registerNetworkComponent<PlayerPodComponent>();
    engine.registerNetworkComponent<AIBehaviorComponent>();
    engine.registerNetworkComponent<BossComponent>();
    engine.registerNetworkComponent<HealthComponent>();
    engine.registerNetworkComponent<EnemySpawnComponent>();
}

int main() {
    GameEngine engine;
    GameManager gm;

    setupPrediction(engine, gm);
    engine.setInitFunction([&gm](Environment& env, InputManager& inputs) { gm.init(env, inputs); });

    engine.setLoopFunction([&gm](Environment& env, InputManager& inputs) { gm.update(env, inputs); });
    engine.run();
    return 0;
}
