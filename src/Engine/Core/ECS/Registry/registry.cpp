#include "registry.hpp"
#include <cstdint>
#include "Components/NetworkComponents.hpp"
#include "Guid/Guid.hpp"

Entity Registry::createEntity() {
    uint32_t entity_id = _nextId++;

    addComponent<NetworkIdentity>(entity_id, {generateRandomGuid(), 0});
    return entity_id;
}

void Registry::destroyEntity(Entity id) {
    for (auto& [type, pool] : _pools) {
        if (pool->has(id))
            pool->removeId(id);
    }
    _deadEntities.push_back(id);
    return;
}

Pool_storage& Registry::getComponentPools()
{
    return _pools;
}
