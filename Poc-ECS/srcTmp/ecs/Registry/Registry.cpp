#include "Registry.hpp"

Entity Registry::createEntity()
{
    return _nextId++;
}

void Registry::destroyEntity(Entity id)
{
    for (auto& [type, pool] : _pools) {
        if (pool->has(id))
            pool->removeId(id);
    }
    _deadEntities.push_back(id);
    return;
}