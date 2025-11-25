
#pragma once

#include "../Registry/Registry.hpp"

class ISystem {
    public:
        virtual ~ISystem() = default;

        virtual void init(Registry& registry) = 0;

        virtual void update(Registry& registry, float dt) = 0;
};