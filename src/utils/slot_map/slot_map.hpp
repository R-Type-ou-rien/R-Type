#pragma once

#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <vector>

template <typename data_type>
struct handle_t {
    uint32_t id;
    uint32_t generation;

    bool operator==(const handle_t& handle) const
    {
        return id == handle.id && generation == handle.generation;
    }

    bool operator!=(const handle_t& handle) const
    {
        return !(*this == handle);
    }

    static handle_t Null()
    {
        return {std::numeric_limits<uint32_t>::max(), 0};
    }
};

template<typename data_type>
class SlotMap {
    private:
        struct data_t {
            data_type data;
            uint32_t sparse_index;
        };
   
        struct slot_t {
            uint32_t dense_index = 0;
            uint32_t generation = 0;
        };

        std::vector<slot_t> _sparse;
        std::vector<data_t> _dense;
        std::vector<uint32_t> _freeId;

    public:
        
        handle_t<data_type> insert(const data_type& data)
        {
            uint32_t index;

            if (!_freeId.empty()) {
                index = _freeId.back();
                _freeId.pop_back();
            } else {
                index = _sparse.size();
                _sparse.push_back({0, 0});
            }

            slot_t& slot = _sparse[index];
            slot.dense_index = _dense.size();
            slot.generation = slot.generation;

            _dense.push_back({data, index});
            
            return {index, slot.generation};
        } 

        bool has(handle_t<data_type> handle)
        {
            if (handle.id >= _sparse.size()) {
                return false;
            }

            if (handle.generation != _sparse[handle.id].generation) {
                return false;
            }
            return true;
        }

        std::optional<std::reference_wrapper<data_type>> get_data(handle_t<data_type> handle)
        {
            if (handle.id >= _sparse.size()) {
                return std::nullopt;
            }

            if (handle.generation != _sparse[handle.id].generation) {
                return std::nullopt;
            }

            return std::ref(_dense[_sparse[handle.id].dense_index].data);
        }

        void remove(handle_t<data_type> handle)
        {
            if (!get_data(handle).has_value())
                return;

            slot_t& slotToRemove = _sparse[handle.id];
            uint32_t indexInDense = slotToRemove.dense_index;
            
            uint32_t lastDenseIdx = _dense.size() - 1;
            data_t& lastData = _dense.back();
            
            if (indexInDense != lastDenseIdx) {
                _dense[indexInDense] = lastData;
                _sparse[lastData.sparse_index].dense_index = indexInDense;
            }

            _dense.pop_back();
            slotToRemove.generation++;
            slotToRemove.dense_index = -1;
            _freeId.push_back(handle.id);
        }

        const std::vector<data_t>& get_data_list() const
        {
            return _dense;
        }
};