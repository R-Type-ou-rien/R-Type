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

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: ./r-type-client <server_ip>" << std::endl;
        return 84;
    }

    std::string ip = argv[1];
    ClientGameEngine cl(ip);
    GameManager gm;

        cl.setInitFunction(initInput);
        
        cl.run();
        return 0;
    }
