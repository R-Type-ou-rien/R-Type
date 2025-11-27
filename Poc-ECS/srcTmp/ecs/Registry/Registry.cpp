#include "Registry.hpp"

Entity Registry::createEntity()
{
    return _nextId++;
}

void Registry::destroyEntity(Entity id)
{
    for (auto pool : _pools) {
        if (pool.second->has(id))
            pool.second->remove(id);
    }
    return;
}