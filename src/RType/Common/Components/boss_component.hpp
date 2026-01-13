#pragma once

struct BossWeakPointComponent {
    static constexpr auto name = "BossWeakPointComponent";

    int boss_entity_id;
    int weak_point_index;
    bool is_destroyed = false;
    float offset_x = 0.0f;  // Offset par rapport au boss
    float offset_y = 0.0f;
};

// Sous-entités du boss (tentacules, canons)
struct BossSubEntityComponent {
    static constexpr auto name = "BossSubEntityComponent";

    enum SubEntityType { TENTACLE, CANNON, SHIELD };

    int boss_entity_id;
    SubEntityType type;
    int sub_entity_index;  // 0-3 pour 4 tentacules par exemple
    bool is_active = true;
    bool is_destroyed = false;

    // Position relative au boss
    float offset_x = 0.0f;
    float offset_y = 0.0f;

    // Comportement propre
    float fire_timer = 0.0f;
    float fire_rate = 2.0f;
};
