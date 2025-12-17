 

#include "HUDSystem.hpp"
#include "Components/StandardComponents.hpp"
#include "health.hpp"
#include <iomanip>
#include <sstream>

void HUDSystem::update(Registry& registry, system_context context) {
    if (!context.client_id.has_value())
        return;

    uint32_t myID = context.client_id.value();
    int myHP = -1;

    
    auto& networkIdentities = registry.getEntities<NetworkIdentity>();
    for (auto entity : networkIdentities) {
        auto& netId = registry.getComponent<NetworkIdentity>(entity);
        if (netId.owner_user_id == myID) {
            if (registry.hasComponent<HealthComponent>(entity)) {
                myHP = registry.getComponent<HealthComponent>(entity).current_hp;
            }
            break;
        }
    }

    if (myHP == -1)  
        return;

    
    bool uiExists = false;
    auto& textEntities = registry.getEntities<TextComponent>();

    for (auto entity : textEntities) {
        if (registry.hasComponent<TagComponent>(entity)) {
            auto& tag = registry.getComponent<TagComponent>(entity);
            for (const auto& t : tag.tags) {
                if (t == "HP_UI") {
                    uiExists = true;
                    auto& textComp = registry.getComponent<TextComponent>(entity);
                    std::stringstream ss;
                    ss << "HP: " << myHP;
                    textComp.text = ss.str();

                    
                    if (myHP > 50)
                        textComp.color = sf::Color::Green;
                    else if (myHP > 20)
                        textComp.color = sf::Color::Yellow;
                    else
                        textComp.color = sf::Color::Red;
                    break;
                }
            }
        }
        if (uiExists)
            break;
    }

    if (!uiExists) {
        Entity ui = registry.createEntity();
        TextComponent text;
        text.fontPath = "content/fonts/arial.ttf";  
                                                    
        text.characterSize = 30;
        text.x = 10;
        text.y = 10;
        text.color = sf::Color::Green;
        text.text = "HP: " + std::to_string(myHP);

        registry.addComponent<TextComponent>(ui, text);
        registry.addComponent<TagComponent>(ui, {{"HP_UI"}});
    }
}
