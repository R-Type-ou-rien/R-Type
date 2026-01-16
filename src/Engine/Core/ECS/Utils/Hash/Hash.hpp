#pragma once

#include <cstddef>
#include <cstdint>
namespace Hash {
constexpr uint32_t val_32_const = 0x811c9dc5;
constexpr uint32_t prime_32_const = 0x1000193;

constexpr uint32_t fnv1a(const char* str, const uint32_t hash_value = val_32_const) noexcept {
    return (*str == 0) ? hash_value : fnv1a(str + 1, (hash_value ^ static_cast<uint32_t>(*str)) * prime_32_const);
}
};  // namespace Hash
