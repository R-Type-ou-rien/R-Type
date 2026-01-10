#include <iostream>
#include <ostream>

#include "GameEngineConfig.hpp"
#include "Components/StandardComponents.hpp"
#include "Lib/GameManager/GameManager.hpp"

int main() {
    GameEngine engine;
    GameManager gm;

    engine.setInitFunction([&gm](GameEngine& eng, InputManager& inputs) { gm.init(eng, inputs); });

    engine.setLoopFunction([&gm](GameEngine& eng, InputManager& inputs) { gm.update(eng, inputs); });
    engine.run();
    return 0;
}
