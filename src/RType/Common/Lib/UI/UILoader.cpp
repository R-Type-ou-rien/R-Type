#include "UILoader.hpp"
#include <fstream>
#include <iostream>

void UILoader::loadUI(Environment& env, const std::string& jsonPath) {
    std::ifstream f(jsonPath);
    if (!f.is_open()) {
        return;
    }

    try {
        nlohmann::json data = nlohmann::json::parse(f);
        if (!data.contains("entities"))
            return;

        auto& registry = env.getECS().registry;

        for (const auto& entityData : data["entities"]) {
            Entity entity = registry.createEntity();

            if (entityData.contains("components")) {
                parseComponents(env, entity, entityData["components"]);
            }
        }
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "[UILoader] JSON parse error: " << e.what() << std::endl;
    }
}

void UILoader::parseComponents(Environment& env, Entity entity, const nlohmann::json& components) {
    auto& registry = env.getECS().registry;

    // Transform
    if (components.contains("Transform")) {
        auto& t = components["Transform"];
        transform_component_s transform{};
        transform.x = t.value("x", 0.0f);
        transform.y = t.value("y", 0.0f);
        transform.scale_x = t.value("scale_x", 1.0f);
        transform.scale_y = t.value("scale_y", 1.0f);
        transform.rotation = t.value("rotation", 0.0f);
        registry.addComponent<transform_component_s>(entity, transform);
    }

    // Text
    if (components.contains("Text")) {
        auto& t = components["Text"];
        TextComponent textComp{};
        textComp.text = t.value("text", "Label");
        textComp.fontPath = t.value("font", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf");
        textComp.characterSize = t.value("size", 24);

        if (t.contains("color")) {
            // Simple color parsing, defaulting to white. Implement hex/name parsing if needed.
            std::string colorName = t["color"];
            if (colorName == "red")
                textComp.color = sf::Color::Red;
            else if (colorName == "green")
                textComp.color = sf::Color::Green;
            else if (colorName == "blue")
                textComp.color = sf::Color::Blue;
            else if (colorName == "yellow")
                textComp.color = sf::Color::Yellow;
            else if (colorName == "black")
                textComp.color = sf::Color::Black;
            else
                textComp.color = sf::Color::White;
        }

        textComp.x = t.value("offset_x", 0.0f);
        textComp.y = t.value("offset_y", 0.0f);
        textComp.z_index = t.value("z_index", 100);

        registry.addComponent<TextComponent>(entity, textComp);
    }

    // Sprite2D
    if (components.contains("Sprite2D")) {
        auto& s = components["Sprite2D"];
        std::string texturePath = s.value("texture", "");
        if (!texturePath.empty()) {
            sprite2D_component_s sprite{};
            sprite.handle = env.loadTexture(texturePath);

            if (s.contains("frame")) {
                auto& f = s["frame"];
                if (f.is_array() && f.size() == 4) {
                    sprite.dimension = {f[0].get<float>(), f[1].get<float>(), f[2].get<float>(), f[3].get<float>()};
                }
            }
            sprite.z_index = s.value("z_index", 50);
            registry.addComponent<sprite2D_component_s>(entity, sprite);
        }
    }

    // Clickable
    if (components.contains("Clickable")) {
        auto& c = components["Clickable"];
        ClickableComponent clickable{};
        clickable.bounds.width = c.value("width", 0.0f);
        clickable.bounds.height = c.value("height", 0.0f);
        // x, y usually derived from Transform or explicit offset?
        // Let's assume bounds.x/y are relative to entity transform or 0 if not specified.
        clickable.bounds.x = c.value("x", 0.0f);
        clickable.bounds.y = c.value("y", 0.0f);

        registry.addComponent<ClickableComponent>(entity, clickable);
    }

    // Button
    if (components.contains("Button")) {
        auto& b = components["Button"];
        ButtonComponent button{};
        button.actionName = b.value("action", "");

        button.normalTexturePath = b.value("normalTexture", "");
        if (b.contains("normalFrame")) {
            auto& f = b["normalFrame"];
            button.normalFrame = {f[0], f[1], f[2], f[3]};
        }

        button.hoverTexturePath = b.value("hoverTexture", button.normalTexturePath);
        if (b.contains("hoverFrame")) {
            auto& f = b["hoverFrame"];
            button.hoverFrame = {f[0], f[1], f[2], f[3]};
        } else {
            button.hoverFrame = button.normalFrame;
        }

        button.pressedTexturePath = b.value("pressedTexture", button.normalTexturePath);
        if (b.contains("pressedFrame")) {
            auto& f = b["pressedFrame"];
            button.pressedFrame = {f[0], f[1], f[2], f[3]};
        } else {
            button.pressedFrame = button.normalFrame;
        }

        registry.addComponent<ButtonComponent>(entity, button);
    }
}
