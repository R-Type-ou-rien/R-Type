#pragma once

#include "ISystem.hpp"
#include "Components/StandardComponents.hpp"
#include <functional>
#include <map>
#include <string>

class UISystem : public ISystem {
   public:
    void update(Registry& registry, system_context context) override;

    // Method to bind action names (string) to actual functions
    void bindAction(const std::string& actionName, std::function<void()> callback);

   private:
    std::map<std::string, std::function<void()>> _actions;

    void handleButtonState(Registry& registry, Entity entity, ButtonComponent& button, ClickableComponent& clickable,
                           system_context& context);
    void updateButtonVisuals(Registry& registry, Entity entity, ButtonComponent& button);
};
