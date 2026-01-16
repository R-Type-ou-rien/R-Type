#include <iostream>
#include <ostream>
#include <string>

#include "GameEngineConfig.hpp"
#include "NetworkEngine/NetworkEngine.hpp"
#include "Components/StandardComponents.hpp"
#include "Lib/GameManager/GameManager.hpp"

int main(int ac, char**av) {

    std::string ip = "127.0.0.1";

    if (ac == 2 && av[1])
        ip = std::string(av[1]);

#if defined(SERVER_BUILD)
    GameEngine engine;
#elif defined(CLIENT_BUILD)
    GameEngine engine(ip);
#endif
    GameManager gm;

    engine.setInitFunction([&gm](Environment& env, InputManager& inputs) { gm.init(env, inputs); });

    engine.setLoopFunction([&gm](Environment& env, InputManager& inputs) { gm.update(env, inputs); });
    engine.run();
    return 0;
}
