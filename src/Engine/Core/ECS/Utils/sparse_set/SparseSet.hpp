#include <cstddef>
#include <iostream>
#include <iterator>
#include <ostream>
#include <vector>
#include "ECS/EcsType.hpp"
#include "Components/NetworkComponents.hpp"
#include "Hash/Hash.hpp"
#include "ResourceConfig.hpp"
#include "Context.hpp"

#ifndef SPARSE_HPP
#define SPARSE_HPP

class ISparseSet {
   public:
    virtual ~ISparseSet() = default;
    virtual void removeId(std::size_t Entity) = 0;
    virtual bool has(std::size_t Entity) const = 0;
    virtual void markAsDirty(std::size_t id) = 0;
    virtual std::vector<std::size_t> getUpdatedEntities() = 0;
    virtual std::vector<std::size_t>& getIdList() = 0;  // Returns all entity IDs that have this component
    virtual ComponentPacket createPacket(uint32_t entity, SerializationContext& context) = 0;
    virtual void markAllUpdated() = 0;
    virtual void clearUpdatedEntities() = 0;
    virtual uint32_t getTypeHash() const = 0;  // Returns the hash of the component type
};

template <typename data_type>
class SparseSet : public ISparseSet {
   private:
    std::vector<int> _sparse;
    std::vector<data_type> _dense;
    std::vector<std::size_t> _reverse_dense;
    std::vector<bool> _dirty_dense;

   public:
    void addID(std::size_t id, const data_type& data);
    void removeId(std::size_t id) override;
    bool has(std::size_t id) const override;
    void markAsDirty(std::size_t id) override;
    data_type& getDataFromId(std::size_t id);
    const data_type& getConstDataFromId(std::size_t id);
    std::vector<data_type>& getDataList();
    std::vector<std::size_t>& getIdList();
    std::vector<std::size_t> getUpdatedEntities() override;
    ComponentPacket createPacket(uint32_t entity, SerializationContext& context) override;
    void markAllUpdated() override;
    void clearUpdatedEntities() override;
    uint32_t getTypeHash() const override { return Hash::fnv1a(data_type::name); }
};

template <typename data_type>
void SparseSet<data_type>::addID(std::size_t id, const data_type& data) {
    // Safety: avoid gigantic resizes if an invalid entity id is used.
    // The engine defines MAX_ENTITIES; ids beyond that indicate a logic/network bug.
    if (id >= MAX_ENTITIES) {
        std::cerr << "[SparseSet] Refusing to add component to entity id=" << id << " (MAX_ENTITIES=" << MAX_ENTITIES
                  << ")" << std::endl;
        return;
    }
    if (id >= _sparse.size()) {
        _sparse.resize(id + 1, -1);
    }
    if (_sparse[id] != -1) {
        _dense[_sparse[id]] = data;
        _dirty_dense[_sparse[id]] = true;
        return;
    }
    _sparse[id] = _dense.size();
    _dense.push_back(data);
    _dirty_dense.push_back(true);
    _reverse_dense.push_back(id);
    return;
}

template <typename data_type>
void SparseSet<data_type>::removeId(std::size_t id) {
    if (!has(id)) {
        std::cerr << "Error: removeId: " << id << " does not have any components from this type." << std::endl;
        return;
    }
    std::size_t indexToRemove = _sparse[id];
    std::size_t lastIndex = _dense.size() - 1;
    if (indexToRemove != lastIndex) {
        data_type lastData = _dense[lastIndex];
        std::size_t lastEntity = _reverse_dense[lastIndex];

        _dense[indexToRemove] = lastData;
        _reverse_dense[indexToRemove] = lastEntity;
        _sparse[lastEntity] = indexToRemove;
        _dirty_dense[indexToRemove] = _dirty_dense[lastIndex];
    }
    _sparse[id] = -1;
    _dense.pop_back();
    _dirty_dense.pop_back();
    _reverse_dense.pop_back();

    return;
}

template <typename data_type>
bool SparseSet<data_type>::has(std::size_t id) const {
    if (id < _sparse.size() && _sparse[id] > -1)
        return true;
    return false;
}

template <typename data_type>
data_type& SparseSet<data_type>::getDataFromId(std::size_t id) {
    if (!has(id)) {
        throw std::runtime_error("Entity does not have component!");
    }
    return _dense[_sparse[id]];
}

template <typename data_type>
const data_type& SparseSet<data_type>::getConstDataFromId(std::size_t id) {
    if (!has(id)) {
        throw std::runtime_error("Entity does not have component!");
    }
    return _dense[_sparse[id]];
}

template <typename data_type>
std::vector<data_type>& SparseSet<data_type>::getDataList() {
    return _dense;
}

template <typename data_type>
std::vector<std::size_t>& SparseSet<data_type>::getIdList() {
    return _reverse_dense;
}

template <typename data_type>
void SparseSet<data_type>::markAsDirty(std::size_t id) {
    if (has(id)) {
        _dirty_dense[_sparse[id]] = true;
    }
}

template <typename data_type>
std::vector<std::size_t> SparseSet<data_type>::getUpdatedEntities() {
    std::vector<std::size_t> updated_entities;

    for (std::size_t i = 0; i < _dirty_dense.size(); ++i) {
        if (_dirty_dense[i]) {
            updated_entities.push_back(_reverse_dense[i]);
            _dirty_dense[i] = false;
        }
    }
    return updated_entities;
}

template <typename data_type>
void SparseSet<data_type>::markAllUpdated() {
    for (std::size_t i = 0; i < _dirty_dense.size(); ++i) {
        _dirty_dense[i] = true;
    }
}

template <typename data_type>
void SparseSet<data_type>::clearUpdatedEntities() {
    for (std::size_t i = 0; i < _dirty_dense.size(); ++i) {
        _dirty_dense[i] = false;
    }
}

#include <type_traits>
#include "Components/serialize/serialize.hpp"
#include "Components/serialize/StandardComponents_serialize.hpp"
#include "Components/serialize/tag_component_serialize.hpp"
#include "Components/StandardComponents.hpp"

template <typename data_type>
ComponentPacket SparseSet<data_type>::createPacket(uint32_t entity, SerializationContext& context) {
    ComponentPacket packet;
    packet.entity_guid = entity;
    data_type& comp = getDataFromId(entity);

    packet.component_type = Hash::fnv1a(data_type::name);

    // trouver un autre moyen de check (exemple: verifier s'il y a un handle)
    if constexpr (std::is_same_v<data_type, sprite2D_component_s> || std::is_same_v<data_type, BackgroundComponent>) {
        serialize::serialize(packet.data, comp, context.textureManager);
    } else {
        serialize::serialize(packet.data, comp);
    }
    return packet;
}

#endif
