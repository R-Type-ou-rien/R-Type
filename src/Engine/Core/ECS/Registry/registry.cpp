#include "registry.hpp"
#include <cstdint>
#include "Components/NetworkComponents.hpp"
#include "Guid/Guid.hpp"

Entity Registry::createEntity() {
    uint32_t entity_id;
    if (!_deadEntities.empty()) {
        entity_id = _deadEntities.back();
        _deadEntities.pop_back();
    } else {
        entity_id = _nextId++;
    }

    addComponent<NetworkIdentity>(entity_id, {generateRandomGuid(), 0});
    return entity_id;
}

void Registry::destroyEntity(Entity id) {
    std::cout << "[REGISTRY DEBUG] Destroying entity " << id << std::endl;
    for (auto& [type, pool] : _pools) {
        if (pool->has(id))
            pool->removeId(id);
    }
    _deadEntities.push_back(id);
    std::cout << "[REGISTRY DEBUG] Entity " << id << " recycled (deadEntities size=" << _deadEntities.size() << ")" << std::endl;
    return;
}

Pool_storage& Registry::getComponentPools() {
    return _pools;
}
