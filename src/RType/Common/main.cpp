#include <iostream>
#include <ostream>

#include "GameEngineConfig.hpp"
#include "NetworkEngine/NetworkEngine.hpp"
#include "Components/StandardComponents.hpp"
#include "Lib/GameManager/GameManager.hpp"

int main() {
    GameEngine engine;
    GameManager gm;

    engine.setInitFunction([&gm](Environment& env, InputManager& inputs) { gm.init(env, inputs); });

    engine.setLoopFunction([&gm](Environment& env, InputManager& inputs) { gm.update(env, inputs); });
    engine.run();
    return 0;
}
