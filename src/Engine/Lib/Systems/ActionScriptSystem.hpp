#pragma once
#include <vector>

#include "Components/StandardComponents.hpp"
#include "ISystem.hpp"

class ActionScriptSystem : public ISystem {
   public:
    ActionScriptSystem() = default;
    ~ActionScriptSystem() = default;
    void update(Registry& registry, system_context context);
};
