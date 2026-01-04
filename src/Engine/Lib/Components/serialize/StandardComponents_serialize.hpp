#pragma once

#include "Components/StandardComponents.hpp"
#include "serialize.hpp"
#include "ResourceConfig.hpp"

namespace serialize {

    /** Pattern Component */
    inline void serialize(std::vector<uint8_t>& buffer, const PatternComponent& component) {
        serialize(buffer, component.type);
        serialize(buffer, static_cast<uint32_t>(component.waypoints.size()));
        for (const auto& pair : component.waypoints) {
            serialize(buffer, pair.first);
            serialize(buffer, pair.second);
        }
        serialize(buffer, component.current_index);
        serialize(buffer, component.speed);
        serialize(buffer, component.loop);
        serialize(buffer, component.is_active);
        serialize(buffer, component.amplitude);
        serialize(buffer, component.frequency);
        serialize(buffer, component.time_elapsed);
    }

    inline PatternComponent deserialize_pattern_component(const std::vector<uint8_t>& buffer, size_t& offset) {
        PatternComponent component;
        component.type = deserialize<PatternComponent::PatternType>(buffer, offset);
        uint32_t waypoints_size = deserialize<uint32_t>(buffer, offset);
        component.waypoints.resize(waypoints_size);
        for (uint32_t i = 0; i < waypoints_size; ++i) {
            component.waypoints[i].first = deserialize<float>(buffer, offset);
            component.waypoints[i].second = deserialize<float>(buffer, offset);
        }
        component.current_index = deserialize<int>(buffer, offset);
        component.speed = deserialize<float>(buffer, offset);
        component.loop = deserialize<bool>(buffer, offset);
        component.is_active = deserialize<bool>(buffer, offset);
        component.amplitude = deserialize<float>(buffer, offset);
        component.frequency = deserialize<float>(buffer, offset);
        component.time_elapsed = deserialize<float>(buffer, offset);
        return component;
    }

    /** Resource Component */
    inline void serialize(std::vector<uint8_t>& buffer, const ResourceStat& stat) {
        serialize(buffer, stat.current);
        serialize(buffer, stat.max);
        serialize(buffer, stat.regenRate);
    }

    inline ResourceStat deserialize_resource_stat(const std::vector<uint8_t>& buffer, size_t& offset) {
        ResourceStat stat;
        stat.current = deserialize<float>(buffer, offset);
        stat.max = deserialize<float>(buffer, offset);
        stat.regenRate = deserialize<float>(buffer, offset);
        return stat;
    }

    inline void serialize(std::vector<uint8_t>& buffer, const ResourceComponent& component) {
        serialize(buffer, component.resources);
    }

    inline ResourceComponent deserialize_resource_component(const std::vector<uint8_t>& buffer, size_t& offset) {
        ResourceComponent component;
        component.resources = deserialize_map<std::string, ResourceStat>(buffer, offset);
        return component;
    }

    /** Transform Component */
    inline void serialize(std::vector<uint8_t>& buffer, const transform_component_s& component) {
        serialize(buffer, component.x);
        serialize(buffer, component.y);
        serialize(buffer, component.scale_x);
        serialize(buffer, component.scale_y);
        serialize(buffer, component.rotation);
    }

    inline transform_component_s deserialize_transform_component(const std::vector<uint8_t>& buffer, size_t& offset) {
        transform_component_s component;
        component.x = deserialize<float>(buffer, offset);
        component.y = deserialize<float>(buffer, offset);
        component.scale_x = deserialize<float>(buffer, offset);
        component.scale_y = deserialize<float>(buffer, offset);
        component.rotation = deserialize<float>(buffer, offset);
        return component;
    }

    /** Velocity2D Component */
    inline void serialize(std::vector<uint8_t>& buffer, const Velocity2D& component) {
        serialize(buffer, component.vx);
        serialize(buffer, component.vy);
    }

    inline Velocity2D deserialize_velocity_2d(const std::vector<uint8_t>& buffer, size_t& offset) {
        Velocity2D component;
        component.vx = deserialize<float>(buffer, offset);
        component.vy = deserialize<float>(buffer, offset);
        return component;
    }
    
    /** BoxCOllision Component */
    inline void serialize(std::vector<uint8_t>& buffer, const BoxCollisionComponent& component) {
        serialize(buffer, component.tagCollision);
    }

    inline BoxCollisionComponent deserialize_box_collision_component(const std::vector<uint8_t>& buffer, size_t& offset) {
        BoxCollisionComponent component;
        component.tagCollision = deserialize_vector<std::string>(buffer, offset);
        return component;
    }

    
    /** Sprite2D Component */
    inline void serialize(std::vector<uint8_t>& buffer, const rect& r) {
        serialize(buffer, r.x);
        serialize(buffer, r.y);
        serialize(buffer, r.width);
        serialize(buffer, r.height);
    }

    inline rect deserialize_rect(const std::vector<uint8_t>& buffer, size_t& offset) {
        rect r;
        r.x = deserialize<float>(buffer, offset);
        r.y = deserialize<float>(buffer, offset);
        r.width = deserialize<float>(buffer, offset);
        r.height = deserialize<float>(buffer, offset);
        return r;
    }

    inline void serialize(std::vector<uint8_t>& buffer, const sprite2D_component_s& component, ResourceManager<TextureAsset>& resourceManager) {
        auto name = resourceManager.get_name(component.handle);
        if (name) {
            serialize(buffer, name.value());
        } else {
            serialize(buffer, std::string(""));
        }
        serialize(buffer, component.dimension);
        serialize(buffer, component.is_animated);
        serialize(buffer, component.frames);
        serialize(buffer, component.reverse_animation);
        serialize(buffer, component.loop_animation);
        serialize(buffer, component.animation_speed);
        serialize(buffer, component.current_animation_frame);
        serialize(buffer, component.last_animation_update);
        serialize(buffer, component.z_index);
        serialize(buffer, component.lastUpdateTime);
    }

    inline sprite2D_component_s deserialize_sprite_2d_component(const std::vector<uint8_t>& buffer, size_t& offset, ResourceManager<TextureAsset>& resourceManager) {
        sprite2D_component_s component;
        std::string name = deserialize<std::string>(buffer, offset);
        auto handle = resourceManager.get_handle(name);
        if (handle) {
            component.handle = handle.value();
        }
        component.dimension = deserialize_rect(buffer, offset);
        component.is_animated = deserialize<bool>(buffer, offset);
        uint32_t frames_size = deserialize<uint32_t>(buffer, offset);
        component.frames.resize(frames_size);
        for (uint32_t i = 0; i < frames_size; ++i) {
            component.frames[i] = deserialize_rect(buffer, offset);
        }
        component.reverse_animation = deserialize<bool>(buffer, offset);
        component.loop_animation = deserialize<bool>(buffer, offset);
        component.animation_speed = deserialize<float>(buffer, offset);
        component.current_animation_frame = deserialize<int>(buffer, offset);
        component.last_animation_update = deserialize<float>(buffer, offset);
        component.z_index = deserialize<int>(buffer, offset);
        component.lastUpdateTime = deserialize<float>(buffer, offset);
        return component;
    }

    /** Tag Component */
    inline void serialize(std::vector<uint8_t>& buffer, const TagComponent& component) {
        serialize(buffer, component.tags);
    }

    inline TagComponent deserialize_tag_component(const std::vector<uint8_t>& buffer, size_t& offset) {
        TagComponent component;
        component.tags = deserialize_vector<std::string>(buffer, offset);
        return component;
    }

    /** Text Component */
    inline void serialize(std::vector<uint8_t>& buffer, const sf::Color& color) {
        serialize(buffer, color.r);
        serialize(buffer, color.g);
        serialize(buffer, color.b);
        serialize(buffer, color.a);
    }

    inline sf::Color deserialize_color(const std::vector<uint8_t>& buffer, size_t& offset) {
        sf::Color color;
        color.r = deserialize<uint8_t>(buffer, offset);
        color.g = deserialize<uint8_t>(buffer, offset);
        color.b = deserialize<uint8_t>(buffer, offset);
        color.a = deserialize<uint8_t>(buffer, offset);
        return color;
    }

    inline void serialize(std::vector<uint8_t>& buffer, const TextComponent& component) {
        serialize(buffer, component.text);
        serialize(buffer, component.fontPath);
        serialize(buffer, component.characterSize);
        serialize(buffer, component.color);
        serialize(buffer, component.x);
        serialize(buffer, component.y);
    }

    inline TextComponent deserialize_text_component(const std::vector<uint8_t>& buffer, size_t& offset) {
        TextComponent component;
        component.text = deserialize<std::string>(buffer, offset);
        component.fontPath = deserialize<std::string>(buffer, offset);
        component.characterSize = deserialize<unsigned int>(buffer, offset);
        component.color = deserialize_color(buffer, offset);
        component.x = deserialize<float>(buffer, offset);
        component.y = deserialize<float>(buffer, offset);
        return component;
    }

    /** Projectile Component */
    inline void serialize(std::vector<uint8_t>& buffer, const Projectile& component) {
        serialize(buffer, component.lifetime);
    }

    inline Projectile deserialize_projectile_component(const std::vector<uint8_t>& buffer, size_t& offset) {
        Projectile component;
        component.lifetime = deserialize<float>(buffer, offset);
        return component;
    }

    /** Scroll Component */
    inline void serialize(std::vector<uint8_t>& buffer, const Scroll& component) {
        serialize(buffer, component.scroll_speed_x);
        serialize(buffer, component.scroll_speed_y);
        serialize(buffer, component.is_paused);
    }

    inline Scroll deserialize_scroll_component(const std::vector<uint8_t>& buffer, size_t& offset) {
        Scroll component;
        component.scroll_speed_x = deserialize<float>(buffer, offset);
        component.scroll_speed_y = deserialize<float>(buffer, offset);
        component.is_paused = deserialize<bool>(buffer, offset);
        return component;
    }

    /** Bakcgroud Component */
    inline void serialize(std::vector<uint8_t>& buffer, const BackgroundComponent& component, ResourceManager<TextureAsset>& resourceManager) {
        auto name = resourceManager.get_name(component.texture_handle);
        if (name) {
            serialize(buffer, name.value());
        } else {
            serialize(buffer, std::string(""));
        }
        serialize(buffer, component.x_offset);
        serialize(buffer, component.scroll_speed);
    }

    inline BackgroundComponent deserialize_background_component(const std::vector<uint8_t>& buffer, size_t& offset, ResourceManager<TextureAsset>& resourceManager) {
        BackgroundComponent component;
        std::string name = deserialize<std::string>(buffer, offset);
        auto handle = resourceManager.get_handle(name);
        if (handle) {
            component.texture_handle = handle.value();
        }
        component.x_offset = deserialize<float>(buffer, offset);
        component.scroll_speed = deserialize<float>(buffer, offset);
        return component;
    }
}
