/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** SparseSet.hpp
*/

#pragma once

#include <vector>

#include "../ecs/EcsType.hpp"

class ISparseSet {
    public:
        virtual ~ISparseSet() = default;
        virtual void remove(EntityID entity) = 0;
        virtual bool has(EntityID entity) const = 0;
};

template<typename T>
class SparseSet: public ISparseSet
{
    public:
        SparseSet() { _sparse.resize(MAX_ENTITIES, -1); }

        ~SparseSet() = default;

        void add(EntityID entity, const T& component) {
            if (entity >= _sparse.size()) {
                _sparse.resize(entity + 1, -1);
            }

            _sparse[entity] =  _components.size();
            _denseToEntity.push_back(entity);
            _components.push_back(component);
        }

        void remove(EntityID entity) override {
            if (!has(entity))
                return;

            size_t indexRemoved = _sparse[entity];
            size_t indexLast = _components.size() - 1;
            EntityID entityLast = _denseToEntity[indexLast];

            _components[indexRemoved] = _components[indexLast];
            _denseToEntity[indexRemoved] = entityLast;
            _sparse[entityLast] = indexRemoved;
            _sparse[entity] = -1;
            _components.pop_back();
            _denseToEntity.pop_back();
        }

        bool has(EntityID entity) const override {
            return entity < _sparse.size() && _sparse[entity] != -1;
        }

        T& get(EntityID entity) {
            return _components[_sparse[entity]];
        }

        std::vector<T>& getData() { return _components;}
        
        const std::vector<EntityID>& getEntities() const {
            return _denseToEntity;
        }

    private:
    std::vector<T> _components;
    std::vector<EntityID> _denseToEntity;
    std::vector<size_t> _sparse;
};