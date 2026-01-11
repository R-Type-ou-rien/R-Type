#pragma once

#include <vector>
#include <cstdint>
#include "StandardComponents_serialize.hpp"
#include "tag_component_serialize.hpp"

namespace serialize {

/**
 * @brief Trait de base pour la désérialisation des composants réseau.
 * La signature est identique pour tous les composants afin de permettre
 * l'automatisation dans le GameEngine.
 */
template <typename T>
struct ComponentTraits {
    static T deserialize(const std::vector<uint8_t>& buffer, size_t& offset,
                         ResourceManager<TextureAsset>& resourceManager) {
        // Par défaut, on utilise la fonction générique de serialize.hpp
        return serialize::deserialize<T>(buffer, offset);
    }
};

// --- SPÉCIALISATIONS POUR LES COMPOSANTS STANDARDS ---

template <>
struct ComponentTraits<PatternComponent> {
    static PatternComponent deserialize(const std::vector<uint8_t>& buffer, size_t& offset,
                                        ResourceManager<TextureAsset>&) {
        return deserialize_pattern_component(buffer, offset);
    }
};

template <>
struct ComponentTraits<HealthComponent> {
    static HealthComponent deserialize(const std::vector<uint8_t>& buffer, size_t& offset,
                                       ResourceManager<TextureAsset>&) {
        return deserialize_health_component(buffer, offset);
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
struct ComponentTraits<BoxCollisionComponent> {
    static BoxCollisionComponent deserialize(const std::vector<uint8_t>& buffer, size_t& offset,
                                             ResourceManager<TextureAsset>&) {
        return deserialize_box_collision_component(buffer, offset);
    }
};

template <>
struct ComponentTraits<TagComponent> {
    static TagComponent deserialize(const std::vector<uint8_t>& buffer, size_t& offset,
                                    ResourceManager<TextureAsset>&) {
        return deserialize_tag_component(buffer, offset);
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
struct ComponentTraits<ProjectileComponent> {
    static ProjectileComponent deserialize(const std::vector<uint8_t>& buffer, size_t& offset,
                                           ResourceManager<TextureAsset>&) {
        return deserialize_projectile_component(buffer, offset);
    }
};

template <>
struct ComponentTraits<Scroll> {
    static Scroll deserialize(const std::vector<uint8_t>& buffer, size_t& offset, ResourceManager<TextureAsset>&) {
        return deserialize_scroll_component(buffer, offset);
    }
};

// --- SPÉCIALISATIONS NÉCESSITANT LE RESOURCE MANAGER ---

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
struct ComponentTraits<TeamComponent> {
    static TeamComponent deserialize(const std::vector<uint8_t>& buffer, size_t& offset,
                                     ResourceManager<TextureAsset>&) {
        return deserialize_team_component(buffer, offset);
    }
};

template <>
struct ComponentTraits<DamageOnCollision> {
    static DamageOnCollision deserialize(const std::vector<uint8_t>& buffer, size_t& offset,
                                         ResourceManager<TextureAsset>&) {
        return deserialize_damage_on_collision(buffer, offset);
    }
};

template <>
struct ComponentTraits<NetworkIdentity> {
    static NetworkIdentity deserialize(const std::vector<uint8_t>& buffer, size_t& offset,
                                       ResourceManager<TextureAsset>&) {
        return deserialize_network_identity(buffer, offset);
    }
};

// GameTimerComponent (included via StandardComponents_serialize.hpp)

template <>
struct ComponentTraits<::GameTimerComponent> {
    static ::GameTimerComponent deserialize(const std::vector<uint8_t>& buffer, size_t& offset,
                                            ResourceManager<TextureAsset>&) {
        return deserialize_game_timer_component(buffer, offset);
    }
};

}  // namespace serialize