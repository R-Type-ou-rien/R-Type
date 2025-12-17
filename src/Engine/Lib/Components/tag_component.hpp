#pragma once

#include <cstdint>
#include <vector>

struct CollidedEntity {
    static constexpr auto name = "CollidedEntity";
    std::vector<uint32_t> tags;
};
