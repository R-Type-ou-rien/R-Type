#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include "Components/NetworkComponents.hpp"
#include "ResourceManagerBase.hpp"
#include "slot_map/slot_map.hpp"

template <typename ResourceType>
class ClientResourceManager : public ResourceManagerBase<ClientResourceManager<ResourceType>, ResourceType> {
   private:
    std::unordered_map<uint32_t, handle_t<ResourceType>> _hash_resources;

   public:
    handle_t<ResourceType> load(const std::string& ressource_name, const ResourceType& ressource_data) {
        if (this->_loaded_resource.find(ressource_name) != this->_loaded_resource.cend()) {
            return this->_loaded_resource.find(ressource_name)->second;
        }

        handle_t<ResourceType> newHandle = this->_resource_storage.insert(ressource_data);
        this->_loaded_resource[ressource_name] = newHandle;
        // add in the hash map
        return newHandle;
    }

    void remove(handle_t<ResourceType> handle) {
        if (!this->_resource_storage.has(handle)) {
            return;
        }
        this->_resource_storage.remove(handle);
        return;
    }

    // load from packet
    void update_resources_from_packet(ResourcePacket packet) {}
};
