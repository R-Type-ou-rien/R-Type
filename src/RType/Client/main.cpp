#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <iostream>
#include <ostream>

#include "ClientGameEngine.hpp"
#include "Components/StandardComponents.hpp"
#include "GameManager/GameManager.hpp"

int main() {
    ClientGameEngine cl;
    GameManager gm;

    // cl.setInitFunction([&gm](ECS& ecs){ gm.init(ecs); });
    // cl.setUserFunction([&gm](ECS& ecs){ gm.update(ecs); });  // -> to rename, this function is called in the loop
    cl.run();
    return 0;
}
