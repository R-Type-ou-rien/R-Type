#include "GameEngineConfig.hpp"
#include "Lib/GameManager/GameManager.hpp"

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

    engine.setInitFunction([&gm](std::shared_ptr<Environment> env, InputManager& inputs) { gm.init(env, inputs); });

    engine.setLoopFunction([&gm](std::shared_ptr<Environment> env, InputManager& inputs) { gm.update(env, inputs); });
    engine.run();
    return 0;
}
