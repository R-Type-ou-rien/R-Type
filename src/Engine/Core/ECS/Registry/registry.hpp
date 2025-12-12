#include <cstdint>
#include <memory>
#include <tuple>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "Utils/sparse_set/SparseSet.hpp"

#pragma once

using Entity = uint32_t;

class Registry {
   private:
    Entity _nextId = 0;
    std::unordered_map<std::type_index, std::unique_ptr<ISparseSet>> _pools;
    std::vector<Entity> _deadEntities;

   public:
    Registry() = default;
    ~Registry() = default;

    /**
        A function to create an entity.
        @return The Function
    */
    Entity createEntity();

    /**
        A function to destroy an entity and remove his id from the component pools
        @param Entity id
    */
    void destroyEntity(Entity id);

    /**
        A function to get the component pool from the given type or create it if it doesn't exist
        @return The pool of the corresponding type
    */
    template <typename Component>
    SparseSet<Component>& getPool() {
        std::type_index index(typeid(Component));

        if (_pools.find(index) == _pools.end()) {
            _pools[index] = std::make_unique<SparseSet<Component>>();
        }
        return *static_cast<SparseSet<Component>*>(_pools[index].get());
    }

    /**
        A function to add a component to a given entity
        @param Entity id
        @param Component component
    */
    template <typename Component>
    void addComponent(Entity id, Component component) {
        getPool<Component>().addID(id, component);
        return;
    }

    /**
        A function to remove a component from a given entity
        @param Entity entity
    */
    template <typename Component>
    void removeComponent(Entity entity) {
        getPool<Component>().removeId(entity);
    }

    /**
        A function to get the component of an entity
        @param Entity entity
        @return The function returns a reference to a component
    */
    template <typename Component>
    Component& getComponent(Entity entity) {
        return getPool<Component>().getDataFromId(entity);
    }

    /**
        A function to know if an entity has a component
        @param Entity entity
        @return The function returns a boolean corresponding at the presence of a component in an entity
    */
    template <typename Component>
    bool hasComponent(Entity entity) {
        return getPool<Component>().has(entity);
    }

    /**
        A function to get the list of component
        @return The function returns a vector of all the components from the pool of the given type
    */
    template <typename Component>
    std::vector<Component>& getView() {
        return getPool<Component>().getDataList();
    }

    /**
        A function to get the component pools from the types given
        @return The function returns a tuple of the component pools from the given types
    */
    template <typename... Components>
    std::tuple<SparseSet<Components>&...> view() {
        return std::tuple<SparseSet<Components>&...>(getPool<Components>()...);
    }

    /**
        A function to get the entities from a component pool
    */
    template <typename Component>
    std::vector<std::size_t>& getEntities() {
        return getPool<Component>().getIdList();
    }
};
