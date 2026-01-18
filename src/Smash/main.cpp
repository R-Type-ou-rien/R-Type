#include <iostream>
#include <ostream>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Keyboard.hpp>
#include "../../Engine/Core/ClientGameEngine.hpp"
#include "./GameManager/GameManager.hpp"

int main() {
    ClientGameEngine engine(1000, 800);
    GameManager gameManager;

    engine.setInitFunction([&gameManager](ECS& ecs) { gameManager.init(ecs); });
    engine.setUserFunction([&gameManager](ECS& ecs) { gameManager.update(ecs); });
    engine.run();
    return 0;
}
