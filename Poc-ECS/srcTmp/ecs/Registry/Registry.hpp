#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <ostream>
#include <typeindex>
#include <vector>
#include "../../utils/SparseSet/SparserSet.hpp"

#pragma once

using Entity = uint32_t;

class Registry {
    private:
        std::size_t _nextId = 0;
        std::map<std::type_index, std::unique_ptr<ISparseSet>> _pools;

    public:
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

        template <typename Component>
        bool hasComponent(Entity entity) {
            return getPool<Component>()->has(entity);
        }

        /**
            A function to get the component pool
        */
        template<typename Component>
        SparseSet<Component>* getPool()
        {
            std::type_index index(typeid(Component));

            if (_pools.find(index) == _pools.end()) {
                _pools[index] = std::make_unique<SparseSet<Component>>();
            }
            return _pools[index];
        }

        template<typename Component>
        void addComponent(Entity id, Component component)
        {
            getPool<Component>()->addID(id, component);
            return;
        }

        template<typename Component>
        Component& getComponent(Entity id)
        {
            if (!getPool<Component, Entity>()->has(id)) {
                std::cerr << "Error: view: The given entity doesn't have this component." << std::endl;
                return nullptr;
            }
            return getPool<Component>()->getDataFromId(id);
        }

        template <typename Component>
        std::vector<Component>& getView() {
            return getPool<Component>()->getDataList();
        }

        template<typename Component>
        std::vector<Entity> getEntities()
        {
            return getPool<Component>()->getIdList();
        }
        
};