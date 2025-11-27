/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Registry.hpp
*/

#pragma once

#include <unordered_map>
#include <typeindex>
#include <memory>
#include <iostream>

#include "../../utils/SparseSet.hpp"
#include "../EcsType.hpp"

class Registry {
    public:
        Registry() = default;
        ~Registry() = default;
        
        EntityID createEntity() {
            return _nextId++;
        }
        
        void destroyEntity(EntityID entity) {
            for (auto& [type, pool] : _pools) {
                if (pool->has(entity)) {
                    pool->remove(entity);
                }
            }
        }
        
        template <typename T>
        SparseSet<T>* getPool() {
            std::type_index index(typeid(T));
    
            if (_pools.find(index) == _pools.end()) {
                _pools[index] = std::make_unique<SparseSet<T>>();
            }
            return static_cast<SparseSet<T>*>(_pools[index].get());
        }

        template <typename T>
        void addComponent(EntityID entity, T component) {
            getPool<T>()->add(entity, component);
        }

        template <typename T>
        void removeComponent(EntityID entity) {
            getPool<T>()->remove(entity);
        }

        template <typename T>
        bool hasComponent(EntityID entity) {
            return getPool<T>()->has(entity);
        }

        template <typename T>
        T& getComponent(EntityID entity) {
            return getPool<T>()->get(entity);
        }

        template <typename T>
        std::vector<T>& getView() {
            return getPool<T>()->getData();
        }

        template <typename T>
        const std::vector<EntityID>& getEntities() {
            return getPool<T>()->getEntities();
        }

    private:
        EntityID _nextId = 0;
        std::unordered_map<std::type_index, std::unique_ptr<ISparseSet>> _pools;
};