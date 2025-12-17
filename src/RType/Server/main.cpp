#include "ECS.hpp"
#include "ServerGameEngine.hpp"
#include "Components/StandardComponents.hpp"
#include "GameManager/GameManager.hpp"

int main() {
    ServerGameEngine s;
    GameManager gm;

    s.setInitFunction([&gm](ECS& ecs) { gm.init(ecs); });
    s.setUserFunction([&gm](ECS& ecs) { gm.update(ecs); });
    s.setOnPlayerConnect([&gm, &s](uint32_t id) { gm.onPlayerConnect(s.getECS(), id); });
    s.run();
}