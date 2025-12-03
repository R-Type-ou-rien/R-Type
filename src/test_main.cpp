#include <iostream>
#include <ostream>
#include "ecs/Registry/registry.hpp"
#include "ecs/ECS.hpp"
#include "shoot_feature/shooter.hpp"

const double FIRE_RATE = 1.5;
const double LAST_SHOT = 8;


/** la cmd de cochon pour tester: 
    g++ src/test_main.cpp src/ecs/Registry/registry.cpp src/shoot_feature/shooter.cpp 
*/
int main()
{
    ECS ecs;
    Entity shootId = ecs.registry.createEntity();

    /** BASIC TEST */
    ecs.systems.addSystem<ShooterSystem>();
    ecs.registry.addComponent<ShooterComponent>(shootId, 
        {ShooterComponent::NORMAL, FIRE_RATE, LAST_SHOT});

    if (ecs.registry.hasComponent<ShooterComponent>(shootId)) {
        std::cout << "Component successfuly added to the entity " << shootId << std::endl;
    } else {
        std::cout << "Component failed to be added to the entity " << shootId << std::endl;
        return -1;
    }

    /** TEST STORED VALUES */
    ShooterComponent res = ecs.registry.getComponent<ShooterComponent>(shootId);

    if (res.fire_rate == FIRE_RATE && res.last_shot == LAST_SHOT && res.type == ShooterComponent::NORMAL) {
        std::cout << "Component's values are correct in the component pool" << std::endl;
    } else {
        std::cout << "Component's values are different in the component pool" << std::endl;
        return -1;
    }

    /** SYSTEM UPDATE TEST */
    ecs.systems.updateAll(0.5);

}