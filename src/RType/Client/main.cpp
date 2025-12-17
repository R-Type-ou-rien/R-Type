#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <iostream>
#include <ostream>

#include "ClientGameEngine.hpp"
#include "Components/StandardComponents.hpp"
#include "../../Common/Lib/GameManager/GameManager.hpp"

void initInput(ECS& ecs) {
    ecs.input.bindAction("move_left", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Q});
    ecs.input.bindAction("move_right", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::D});
    ecs.input.bindAction("move_up", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Z});
    ecs.input.bindAction("move_down", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::S});
    ecs.input.bindAction("shoot", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Space});
}

int main() {
    ClientGameEngine cl;
    GameManager gm;

        cl.setInitFunction(initInput);
        // cl.setUserFunction([&gm](ECS& ecs){ gm.update(ecs); });  // -> to rename, this function is called in the loop
        cl.run();
        return 0;
    }
