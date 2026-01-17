#pragma once

#include <string>
#include "registry.hpp"
#include "ISystem.hpp"
#include "../../Components/config.hpp"

class IMobSpawner {
   public:
    virtual ~IMobSpawner() {}
    virtual Entity spawn(Registry& registry, system_context context, float x, float y, const EntityConfig& config) = 0;
    virtual std::string getTypeName() const = 0;
};
