#include "registry.hpp"
#include <random>

uint64_t generateRandomGuid() {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    static std::uniform_int_distribution<uint64_t> dis;
    return dis(gen);
}

Entity Registry::createEntity() {
    Entity id = _nextId++;
    addComponent<NetworkIdentity>(id, NetworkIdentity{generateRandomGuid(), 0});
    return id;
}

void Registry::destroyEntity(Entity id) {
    for (auto& [type, pool] : _pools) {
        if (pool->has(id))
            pool->removeId(id);
    }
    _deadEntities.push_back(id);
    return;
}

Pool_storage& Registry::getPools()
{
    return _pools;
}
