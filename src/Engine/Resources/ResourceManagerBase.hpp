#pragma once

#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include "slot_map/slot_map.hpp"

template<typename Derived, typename ResourceType>
class ResourceManagerBase {
    protected:
        SlotMap<ResourceType> _resource_storage;
        std::unordered_map<std::string, handle_t<ResourceType>> _loaded_resource;

    public:
        handle_t<ResourceType> load(const std::string name, const ResourceType& resource)
        {
            return static_cast<Derived*>(this)->load();
        }

        bool has(handle_t<ResourceType> handle)
        {
            return _resource_storage.has(handle);
        }

        bool is_loaded(const std::string& name)
        {
            if (_loaded_resource.find(name) != _loaded_resource.cend())
                return true;
            return false;
        }

        std::optional<std::reference_wrapper<ResourceType>> get_resource(handle_t<ResourceType> handle)
        {
            return _resource_storage.get_data(handle);
        }

        std::optional<std::reference_wrapper<handle_t<ResourceType>>> get_handle(const std::string& name)
        {
            if (this->_loaded_resource.find(name) != this->_loaded_resource.cend())
                return this->_loaded_resource[name];
            return std::nullopt;
        }

        void remove(handle_t<ResourceType> handle)
        {
            static_cast<Derived*>(this)->remove(handle);
        }

        std::vector<ResourceType> get_all()
        {
            return this->_resource_storage.get_data_list();
        }
};