#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include "Environment/Environment.hpp"
#include "Components/StandardComponents.hpp"

class UILoader {
   public:
    static void loadUI(Environment& env, const std::string& jsonPath);

   private:
    static void parseComponents(Environment& env, Entity entity, const nlohmann::json& components);
};
