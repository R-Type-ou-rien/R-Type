#pragma once

#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <ostream>
#include <tuple>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <typeinfo>

#include "Components/NetworkComponents.hpp"
#include "sparse_set/SparseSet.hpp"
#include "Hash/Hash.hpp"

using Entity = uint32_t;
using Pool_storage = std::unordered_map<std::type_index, std::unique_ptr<ISparseSet>>;

class Registry {
   private:
    Entity _nextId = 0;
    Pool_storage _pools;
    std::vector<Entity> _deadEntities;
    
   public:
    Registry() = default;
    ~Registry() = default;

     
    Entity createEntity();

     
    void destroyEntity(Entity id);

     
    template <typename Component>
    SparseSet<Component>& getPool() {
        std::type_index index(typeid(Component));

        if (_pools.find(index) == _pools.end()) {
            _pools[index] = std::make_unique<SparseSet<Component>>();
        }
        return *static_cast<SparseSet<Component>*>(_pools[index].get());
    }

     
    template <typename Component>
    void addComponent(Entity id, Component component) {
        getPool<Component>().addID(id, component);
        return;
    }

     
    template <typename Component>
    void removeComponent(Entity entity) {
        getPool<Component>().removeId(entity);
    }

     
    template <typename Component>
    Component& getComponent(Entity entity) {
        return getPool<Component>().getDataFromId(entity);
    }

     
    template <typename Component>
    const Component& getComponentConst(Entity entity) {
        return getPool<Component>().getDataFromIdConst(entity);
    }

     
    template <typename Component>
    bool hasComponent(Entity entity) {
        return getPool<Component>().has(entity);
    }

     
    template <typename Component>
    std::vector<Component>& getView() {
        return getPool<Component>().getDataList();
    }

     
    template <typename... Components>
    std::tuple<SparseSet<Components>&...> view() {
        return std::tuple<SparseSet<Components>&...>(getPool<Components>()...);
    }

     
    template <typename Component>
    std::vector<std::size_t>& getEntities() {
        return getPool<Component>().getIdList();
    }

    Pool_storage& getPools();
};
