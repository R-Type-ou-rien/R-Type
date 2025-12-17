#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <ostream>
#include <vector>
#include "Components/NetworkComponents.hpp"
#include "Components/ComponentSerializer.hpp"
#include "Hash/Hash.hpp"

#ifndef SPARSE_HPP

class ISparseSet {
   public:
    virtual ~ISparseSet() = default;
    virtual void removeId(std::size_t Entity) = 0;
    virtual bool has(std::size_t Entity) const = 0;
    virtual std::vector<std::size_t> popUpdatedEntities() = 0;
    virtual ComponentPacket createPacket(uint32_t entity) = 0;
    virtual void markAllUpdated() = 0;
};

template <typename data_type>
class SparseSet : public ISparseSet {
   private:
    /**
        A vector containing the index of the data in the dense vector at the id position.
        The index equal -1 if their is no data
    */
    std::vector<int> _sparse;

    /** A vector containing the data stored contigously */
    std::vector<data_type> _dense;

    /**
        A vector containing id at their data index positions.
        The id are stored contigously
    */
    std::vector<std::size_t> _reverse_dense;

    std::vector<bool> _dirty;

   public:
    /**
        A function to add a new data to the given id
        @param std::size_t id
    */
    void addID(std::size_t id, const data_type& data) {
        if (id >= _sparse.size()) {
            _sparse.resize(id + 1, -1);
        }
        if (_sparse[id] != -1) {
            _dense[_sparse[id]] = data;
            _dirty[_sparse[id]] = true;
            return;
        }
        _sparse[id] = _dense.size();
        _dense.push_back(data);
        _reverse_dense.push_back(id);
        _dirty.push_back(true);
        return;
    }

    /**
        A function to remove a data from the given id
        @param std::size_t id
    */
    void removeId(std::size_t id) override {
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

            _dirty[indexToRemove] = _dirty[lastIndex];
        }
        _sparse[id] = -1;
        _dense.pop_back();
        _reverse_dense.pop_back();
        _dirty.pop_back();
        std::cout << "Component from entity " << id << " successfully removed." << std::endl;
        return;
    }

    /**
        A function to check if an id has a data
        @param std::size_t id
        @return Returns true if the id has a data and false otherwise
    */
    bool has(std::size_t id) const override {
        if (id < _sparse.size() && _sparse[id] > -1)
            return true;
        return false;
    }

    /**
        A function to get the data of a given id
        @param std::size_t id
        @return The function returns reference to the corresponding data
    */
    data_type& getDataFromId(std::size_t id) {
        if (!has(id)) {
            throw std::runtime_error("Entity does not have component");
        }
        _dirty[_sparse[id]] = true;
        return _dense[_sparse[id]];
    }

    /**
        A function to get the data of a given id without marking it as dirty (read-only)
        @param std::size_t id
        @return The function returns const reference to the corresponding data
    */
    const data_type& getDataFromIdConst(std::size_t id) const {
        if (!has(id)) {
            throw std::runtime_error("Entity does not have component");
        }
        return _dense[_sparse[id]];
    }

    /**
        A function to get the data's list stored
        @return The function returns the dense vector
    */
    std::vector<data_type>& getDataList() { return _dense; }

    /**
        A function to get the id's list stored
        @return The function returns the _reverse_dense vector
    */
    std::vector<std::size_t>& getIdList() { return _reverse_dense; }

    std::vector<std::size_t> popUpdatedEntities() override {
        std::vector<std::size_t> updated;
        for (size_t i = 0; i < _dirty.size(); ++i) {
            if (_dirty[i]) {
                updated.push_back(_reverse_dense[i]);
                _dirty[i] = false;
            }
        }
        return updated;
    }

    ComponentPacket createPacket(uint32_t entity) override {
        ComponentPacket packet;
        data_type& comp = this->getDataFromId(entity);

        packet.component_type = Hash::fnv1a(comp.name);
        packet.data = Serializer<data_type>::serialize(comp);
        return packet;
    }

    void markAllUpdated() override {
        for (size_t i = 0; i < _dirty.size(); ++i) {
            _dirty[i] = true;
        }
    }
};

#endif
