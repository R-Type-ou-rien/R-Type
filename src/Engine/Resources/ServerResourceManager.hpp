#pragma once

#include <string>
#include <type_traits>
#include <vector>
#include "Components/NetworkComponents.hpp"
#include "ResourceManagerBase.hpp"

template<typename ResourceType>
class ServerResourceManager : public ResourceManagerBase<ServerResourceManager<ResourceType>, ResourceType> {
    private:
        std::vector<ResourceType> _resources_to_load;
        std::vector<ResourceType> _resources_to_delete;

    public:
        handle_t<ResourceType> load(const std::string& ressource_name, const ResourceType& ressource_data) {
            if (this->_loaded_resource.find(ressource_name) != this->_loaded_resource.cend()) {
                return this->_loaded_resource.find(ressource_name)->second;
            }

            handle_t<ResourceType> newHandle = this->_resource_storage.insert(ressource_data);
            this->_loaded_resource[ressource_name] = newHandle;
            _resources_to_load.push_back(ressource_data);
            return newHandle;
        }

        void remove(handle_t<ResourceType> handle) {
            if (!this->_resource_storage.has(handle)) {
                return;
            }
            // send resource to remove
            _resources_to_delete.push_back(this->get_resource(handle));
            this->_resource_storage.remove(handle);
            return;
        }

        ResourcePacket get_resources_to_load()
        {
            ResourcePacket packet;

            for (auto source : _resources_to_load)
                packet.resources_source.push_back(source);
            _resources_to_load.clear();
            packet.action = ResourceAction::LOAD;
            return packet;
        }

        ResourcePacket get_resources_to_delete()
        {
            ResourcePacket packet;

            for (auto source : _resources_to_delete)
                packet.resources_source.push_back(source);
            _resources_to_delete.clear();
            packet.action = ResourceAction::DELETE;
            // if constexpr (std::is_same<typename, typename>) {
            
            // }
            return packet;
        }
};
