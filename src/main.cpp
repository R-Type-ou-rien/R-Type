#include "ecs/Registry/registry.hpp"
#include <iostream>
#include <ostream>
#include <string>

struct transform {float x; float y;};

int main()
{
    Registry reg;

    int entity1 = reg.createEntity();
    int entity2 = reg.createEntity();
    std::string rep;
    std::cout << "entity1 = " << entity1 << " entity2 = " << entity2 << std::endl;
    auto& pool = reg.getPool<transform>();
    reg.addComponent<transform>(entity2, {3, 4});
    if (reg.hasComponent<transform>(entity2)) {
        rep = "true";
    } else {
        rep = "false";
    }
    std::cout << "Is entity correctly link to transform component: " << rep << std::endl;

    if (!reg.hasComponent<transform>(entity2)) {
        std::cout << "ERROR: COMPONENT NOT ASSIGNED" << std::endl;
        return -1;
    }
    reg.removeComponent<transform>(entity2);
    if (!reg.hasComponent<transform>(entity2)) {
        rep = "true";
    } else {
        rep = "false";
    }
    std::cout << "The entity is now unlinked: " << rep << std::endl;
}