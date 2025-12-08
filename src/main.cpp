#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <iostream>
#include <ostream>

#include "ecs/client/InputSystem/InputSystem.hpp"
#include "ecs/client/RenderSystem/RenderSystem.hpp"
#include "ecs/common/Components/Components.hpp"
#include "ecs/common/ECS.hpp"
#include "ecs/common/ISystem.hpp"
#include "ecs/common/MoveSystem/MoveSystem.hpp"
#include "game_engine/client/client_game_engine/client_game_engine.hpp"

void init(ECS& ecs) {
    Entity player = ecs.registry.createEntity();

    /* link of the entity with components */
    ecs.registry.addComponent(player, transform_component_s{100.f, 300.f});
    ecs.registry.addComponent(player, Velocity2D{0.f, 0.f});

    // contrôle manette
    // ecs.registry.addComponent<GamepadControl>(player, {
    //     0,                      // joystickId (0 = première manette)
    //     sf::Joystick::Axis::X,        // axe horizontal
    //     sf::Joystick::Axis::Y,        // axe vertical
    //     XBOXA,                      // buttonShoot (bouton 0 = A sur Xbox, par ex.)
    //     300.f,                  // speed
    //     15.f                    // deadZone
    // });

    // logique de tir
    ecs.registry.addComponent<Shooter>(player, {
                                                   sf::Keyboard::Key::J,  // shootKey (toujours dispo au clavier)
                                                   800.f,                 // projectileSpeed
                                                   2.0f,                  // projectileLifetime
                                                   3.0f,                  // fireRate
                                                   // 100.f               // timeSinceLastShot
                                               });

    /*
        handle instance, a handle is a 'ticket' to ask to access to a ressource
        when inserting a ressource, the slot map returns a "ticket"
    */
    handle_t<sf::Texture> handle = ecs._textureManager.load_resource("content/sprites/r-typesheet42.gif",
                                                                     sf::Texture("content/sprites/r-typesheet42.gif"));

    /*
        instance of a 2D sprite component
        Look !! it takes a "ticket" to access the texture later !!
    */
    sprite2D_component_s sprite_info;
    sprite_info.handle = handle;
    sprite_info.animation_speed = 0.5f;
    sprite_info.current_animation_frame = 0;
    sprite_info.dimension.position = {0, 0};
    sprite_info.dimension.size = {32, 16};
    sprite_info.z_index = 1;

    /* the component is linked to the player entity */
    ecs.registry.addComponent<sprite2D_component_s>(player, sprite_info);

    // ---- TEST BINDINGS ----
    // Clavier : espace = Action::Shoot
    ecs.input.bindAction(Action::Shoot, InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Space});

    // Clavier : Z = MoveUp
    ecs.input.bindAction(Action::MoveUp, InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Z});

    // Manette : bouton A = Shoot
    InputBinding shootPad;
    shootPad.device = InputDeviceType::GamepadButton;
    shootPad.joystickId = 0;
    shootPad.joystickButton = 0;
    ecs.input.bindAction(Action::Shoot, shootPad);

    // Gâchette droite (RT) = Shoot
    InputBinding rt;
    rt.device = InputDeviceType::GamepadAxis;
    rt.joystickId = 0;
    rt.axis = sf::Joystick::Axis::Z;
    rt.axisThreshold = 20.f;
    rt.axisSign = +1;  // direction positive
    ecs.input.bindAction(Action::Shoot, rt);

    // D-PAD haut = MoveUp
    InputBinding dpadUp;
    dpadUp.device = InputDeviceType::GamepadAxis;
    dpadUp.joystickId = 0;
    dpadUp.axis = sf::Joystick::Axis::PovY;
    dpadUp.axisThreshold = 50.f;
    dpadUp.axisSign = +1;
    ecs.input.bindAction(Action::MoveUp, dpadUp);

    // Stick gauche vers la gauche = MoveLeft
    InputBinding stickLeft;
    stickLeft.device = InputDeviceType::GamepadAxis;
    stickLeft.joystickId = 0;
    stickLeft.axis = sf::Joystick::Axis::X;  // axe horizontal du stick gauche
    stickLeft.axisThreshold = 20.f;          // deadzone
    stickLeft.axisSign = -1;                 // négatif = gauche
    ecs.input.bindAction(Action::MoveLeft, stickLeft);

    // Stick gauche vers la droite = MoveRight
    stickLeft.device = InputDeviceType::GamepadAxis;
    stickLeft.joystickId = 0;
    stickLeft.axis = sf::Joystick::Axis::X;  // axe horizontal du stick gauche
    stickLeft.axisThreshold = 20.f;          // deadzone
    stickLeft.axisSign = +1;                 // positif = droit
    ecs.input.bindAction(Action::MoveRight, stickLeft);

    // Stick gauche vers la bas = MoveDown
    stickLeft.device = InputDeviceType::GamepadAxis;
    stickLeft.joystickId = 0;
    stickLeft.axis = sf::Joystick::Axis::Y;  // axe horizontal du stick gauche
    stickLeft.axisThreshold = 20.f;          // deadzone
    stickLeft.axisSign = +1;                 // positif = haut
    ecs.input.bindAction(Action::MoveDown, stickLeft);

    // Stick gauche vers la haut = MoveUp
    stickLeft.device = InputDeviceType::GamepadAxis;
    stickLeft.joystickId = 0;
    stickLeft.axis = sf::Joystick::Axis::Y;  // axe horizontal du stick gauche
    stickLeft.axisThreshold = 20.f;          // deadzone
    stickLeft.axisSign = -1;                 // négatif = bas
    ecs.input.bindAction(Action::MoveUp, stickLeft);

    std::cout << "=== InputManager Test Ready ===\n";
    std::cout << "Press SPACE or Gamepad A or RT to test Shoot\n";
    std::cout << "Press Z or D-Pad UP to test MoveUp\n";
    std::cout << "Push LEFT on left stick to test MoveLeft\n";
}

void play(ECS& ecs) {
    if (ecs.input.isJustPressed(Action::Shoot))
        std::cout << "[Shoot] just pressed!\n";

    if (ecs.input.isJustReleased(Action::Shoot))
        std::cout << "[Shoot] just released (hold time = " << ecs.input.getState(Action::Shoot).lastReleaseHoldTime
                  << "s)\n";

    if (ecs.input.isLongPress(Action::Shoot, 0.5f))
        std::cout << "[Shoot] LONG PRESS (>= 0.5s)\n";

    if (ecs.input.isShortPress(Action::Shoot, 0.2f))
        std::cout << "[Shoot] SHORT PRESS (< 0.2s)\n";

    if (ecs.input.isPressed(Action::MoveLeft))
        std::cout << "[MoveLeft] pressed\n";

    if (ecs.input.isPressed(Action::MoveRight))
        std::cout << "[MoveRight] pressed\n";

    if (ecs.input.isPressed(Action::MoveUp))
        std::cout << "[MoveUp] pressed\n";

    if (ecs.input.isPressed(Action::MoveDown))
        std::cout << "[MoveDown] pressed\n";

    return;
}

int main() {
    ClientGameEngine cl;

    cl.setInitFunction(init);
    cl.setUserFunction(play);  // -> to rename, this function is called in the loop
    cl.run();
    return 0;
}

// int main() {
//     /* Instance of ECS class */
//     ECS ecs(1920, 1080, "R-Type");

//     /* An instance of a ressource manager */
//     ResourceManager<sf::Texture> texture_manager;

//     /* context instance, a struct containing what systems need  */
//     system_context context = {0, texture_manager};

//     /* system addition */
//     ecs.systems.addSystem<InputSystem>();
//     ecs.systems.addSystem<GamepadInputSystem>();
//     ecs.systems.addSystem<ShootSystem>();
//     ecs.systems.addSystem<MoveSystem>();
//     ecs.systems.addSystem<ProjectileSystem>();
//     ecs.systems.addSystem<RenderSystem>(ecs.getWindow());
//     ecs.systems.addSystem<CooldownSystem>(ecs.getWindow());

//     /* creation of an entity */
//     Entity player = ecs.registry.createEntity();

//     /* link of the entity with components */
//     ecs.registry.addComponent(player, transform_component_s{100.f, 300.f});
//     ecs.registry.addComponent(player, Velocity2D{0.f, 0.f});

//     // contrôle manette
//     ecs.registry.addComponent<GamepadControl>(player, {
//         0,                      // joystickId (0 = première manette)
//         sf::Joystick::Axis::X,        // axe horizontal
//         sf::Joystick::Axis::Y,        // axe vertical
//         XBOXA,                      // buttonShoot (bouton 0 = A sur Xbox, par ex.)
//         300.f,                  // speed
//         15.f                    // deadZone
//     });

//     // logique de tir
//     ecs.registry.addComponent<Shooter>(player, {
//         sf::Keyboard::Key::J,  // shootKey (toujours dispo au clavier)
//         800.f,            // projectileSpeed
//         2.0f,             // projectileLifetime
//         3.0f,            // fireRate
//         // 100.f               // timeSinceLastShot
//     });

//     /*
//         handle instance, a handle is a 'ticket' to ask to access to a ressource
//         when inserting a ressource, the slot map returns a "ticket"
//     */
//     handle_t<sf::Texture> handle = texture_manager.load_resource("content/sprites/r-typesheet42.gif",
//     sf::Texture("content/sprites/r-typesheet42.gif"));

//     /*
//         instance of a 2D sprite component
//         Look !! it takes a "ticket" to access the texture later !!
//     */
//     sprite2D_component_s sprite_info;
//     sprite_info.handle = handle;
//     sprite_info.animation_speed = 0.5f;
//     sprite_info.current_animation_frame = 0;
//     sprite_info.dimension.position = {0, 0};
//     sprite_info.dimension.size =  {32, 16};
//     sprite_info.z_index = 1;

//     /* the component is linked to the player entity */
//     ecs.registry.addComponent<sprite2D_component_s>(player, sprite_info);

//     // /*
//     //     this part is simple sfml and will disapear but it will stay until I change the architecture to test
//     //     you can still notice some changes like the event handing
//     // */
//     // sf::RenderWindow& window = ecs.getWindow();

//     // sf::Clock clock;
//     // while (window.isOpen()) {
//     //     while (const std::optional event = window.pollEvent()) {
//     //         if (event->is<sf::Event::Closed>())
//     //             window.close();
//     //     }
//     //     sf::Time elapsed = clock.restart();
//     //     context.dt = elapsed.asSeconds();
//     //     window.clear();
//     //     ecs.update(context);
//     //     window.display();
//     // }

//     // // ecs.run();
//     // return 0;
// }
