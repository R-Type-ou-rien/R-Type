#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <iostream>
#include <ostream>

#include "ClientGameEngine.hpp"
#include "Components/StandardComponents.hpp"
#include "Lib/GameManager/GameManager.hpp"

void init(ECS& ecs) {
    Entity player = ecs.registry.createEntity();

    ecs.registry.addComponent(player, transform_component_s{100.f, 300.f});
    ecs.registry.addComponent(player, Velocity2D{0.f, 0.f});

    ecs.input.bindAction("Shoot", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Space});

    ecs.input.bindAction("MoveUp", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Z});

    std::cout << "=== InputManager Test Ready ===\n";
    std::cout << "Press SPACE or Gamepad A or RT to test Shoot\n";
    std::cout << "Press Z or D-Pad UP to test MoveUp\n";
    std::cout << "Push LEFT on left stick to test MoveLeft\n";
}

void play(ECS& ecs) {
    return;
}

int main() {
    ClientGameEngine cl;
    GameManager gm;

    cl.setInitFunction([&gm](ECS& ecs) { gm.init(ecs); });
    cl.setUserFunction([&gm](ECS& ecs) { gm.update(ecs); });
    cl.run();
    return 0;
}
