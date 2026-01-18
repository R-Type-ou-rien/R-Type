#include <iostream>
#include <ostream>
#include <type_traits>
#include "ClientGameEngine.hpp"
#include "GameEngineConfig.hpp"
#include "Lib/GameManager/GameManager.hpp"
#include "../../RType/Common/Systems/score.hpp"

template <typename T>
void setupPrediction(T& engine, GameManager& gm) {
    if constexpr (std::is_same_v<T, ClientGameEngine>) {
        engine.setPredictionLogic([&gm](Entity e, Registry& r, const InputSnapshot& inputs, float dt) {
            gm.predictionLogic(e, r, inputs, dt);
        });
    }
}

int main(int argc, char* argv[]) {
    std::string ip = "127.0.0.1";
    if (argc > 1) {
        ip = argv[1];
    }
    GameEngine engine(ip);
    GameManager gm;

#if defined(CLIENT_BUILD)
    gm.setWindow(&engine.getWindow());
    gm.setLocalPlayerId(engine.getClientId());
#endif
    
    setupPrediction(engine, gm);
    engine.registerNetworkComponent<DamageOnCollision>();
    engine.registerNetworkComponent<TeamComponent>();
    engine.registerNetworkComponent<ProjectileComponent>();
    engine.registerNetworkComponent<HealthComponent>();
    engine.registerNetworkComponent<EnemySpawnComponent>();
    engine.registerNetworkComponent<ShooterComponent>();
    engine.registerNetworkComponent<ChargedShotComponent>();
    engine.registerNetworkComponent<PodComponent>();
    engine.registerNetworkComponent<::GameTimerComponent>();
    engine.registerNetworkComponent<PlayerPodComponent>();
    engine.registerNetworkComponent<BehaviorComponent>();
    engine.registerNetworkComponent<BossComponent>();
    engine.registerNetworkComponent<BossSubEntityComponent>();
    engine.registerNetworkComponent<ScoreComponent>();

    engine.setInitFunction([&gm](std::shared_ptr<Environment> env, InputManager& inputs) { gm.init(env, inputs); });

    engine.setLoopFunction([&gm](std::shared_ptr<Environment> env, InputManager& inputs) { gm.update(env, inputs); });
    engine.run();
    return 0;
}
