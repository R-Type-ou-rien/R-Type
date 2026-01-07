#pragma once

#include "StandardComponents_serialize.hpp"

namespace serialize {

template <typename T>
struct ComponentTraits {
    static T deserialize(const std::vector<uint8_t>& buffer, size_t& offset,
                         ResourceManager<TextureAsset>& resourceManager);
};

// Specializations

template <>
struct ComponentTraits<StateComponent> {
    static StateComponent deserialize(const std::vector<uint8_t>& buffer, size_t& offset,
                                      ResourceManager<TextureAsset>&) {
        return deserialize_state_component(buffer, offset);
    }
};

template <>
struct ComponentTraits<transform_component_s> {
    static transform_component_s deserialize(const std::vector<uint8_t>& buffer, size_t& offset,
                                             ResourceManager<TextureAsset>&) {
        return deserialize_transform_component(buffer, offset);
    }
};

template <>
struct ComponentTraits<Velocity2D> {
    static Velocity2D deserialize(const std::vector<uint8_t>& buffer, size_t& offset, ResourceManager<TextureAsset>&) {
        return deserialize_velocity_2d(buffer, offset);
    }
};

template <>
struct ComponentTraits<sprite2D_component_s> {
    static sprite2D_component_s deserialize(const std::vector<uint8_t>& buffer, size_t& offset,
                                            ResourceManager<TextureAsset>& resourceManager) {
        return deserialize_sprite_2d_component(buffer, offset, resourceManager);
    }
};

template <>
struct ComponentTraits<BackgroundComponent> {
    static BackgroundComponent deserialize(const std::vector<uint8_t>& buffer, size_t& offset,
                                           ResourceManager<TextureAsset>& resourceManager) {
        return deserialize_background_component(buffer, offset, resourceManager);
    }
};

template <>
struct ComponentTraits<ResourceComponent> {
    static ResourceComponent deserialize(const std::vector<uint8_t>& buffer, size_t& offset,
                                         ResourceManager<TextureAsset>&) {
        return deserialize_resource_component(buffer, offset);
    }
};

template <>
struct ComponentTraits<TextComponent> {
    static TextComponent deserialize(const std::vector<uint8_t>& buffer, size_t& offset,
                                     ResourceManager<TextureAsset>&) {
        return deserialize_text_component(buffer, offset);
    }
};

template <>
struct ComponentTraits<BoxCollisionComponent> {
    static BoxCollisionComponent deserialize(const std::vector<uint8_t>& buffer, size_t& offset,
                                             ResourceManager<TextureAsset>&) {
        return deserialize_box_collision_component(buffer, offset);
    }
};

template <>
struct ComponentTraits<PatternComponent> {
    static PatternComponent deserialize(const std::vector<uint8_t>& buffer, size_t& offset,
                                        ResourceManager<TextureAsset>&) {
        return deserialize_pattern_component(buffer, offset);
    }
};

template <>
struct ComponentTraits<Projectile> {
    static Projectile deserialize(const std::vector<uint8_t>& buffer, size_t& offset, ResourceManager<TextureAsset>&) {
        return deserialize_projectile_component(buffer, offset);
    }
};

template <>
struct ComponentTraits<Scroll> {
    static Scroll deserialize(const std::vector<uint8_t>& buffer, size_t& offset, ResourceManager<TextureAsset>&) {
        return deserialize_scroll_component(buffer, offset);
    }
};

template <>
struct ComponentTraits<TagComponent> {
    static TagComponent deserialize(const std::vector<uint8_t>& buffer, size_t& offset,
                                    ResourceManager<TextureAsset>&) {
        return deserialize_tag_component(buffer, offset);
    }
};

}  // namespace serialize
