#pragma once

#include "registry.hpp"

struct LastDamageDealerComponent {
    static constexpr auto name = "LastDamageDealerComponent";
    Entity dealer_entity = -1;
};
