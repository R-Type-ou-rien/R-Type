#pragma once

#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "ECS/Utils/slot_map/slot_map.hpp"

template <typename Resource>
class ResourceManager {
   private:
    SlotMap<Resource> _ressourceStorage;
    std::unordered_map<std::string, handle_t<Resource>> _loaded_data;

   public:
    handle_t<Resource> load_resource(const std::string& ressource_name, const Resource& ressource_data) {
        if (_loaded_data.find(ressource_name) != _loaded_data.cend()) {
            return _loaded_data.find(ressource_name)->second;
        }

        handle_t<Resource> newHandle = _ressourceStorage.insert(ressource_data);
        _loaded_data[ressource_name] = newHandle;
        return newHandle;
    }

    bool has_resource(const handle_t<Resource> handle) { return _ressourceStorage.has(handle); }

    bool is_loaded(const std::string resource_name) {
        if (_loaded_data.find(resource_name) != _loaded_data.cend())
            return true;
        return false;
    }

    std::optional<std::reference_wrapper<Resource>> get_resource(const handle_t<Resource> handle) {
        return _ressourceStorage.get_data(handle);
    }

    std::optional<handle_t<Resource>> get_handle(const std::string resource_name) {
        if (_loaded_data.find(resource_name) != _loaded_data.cend())
            return _loaded_data[resource_name];
        return std::nullopt;
    }

    void remove_resource(handle_t<Resource> handle) {
        if (!_ressourceStorage.has(handle)) {
            return;
        }
        _ressourceStorage.remove(handle);
        return;
    }

    std::vector<Resource> get_all_resources() { return _ressourceStorage.get_data_list(); }
};
