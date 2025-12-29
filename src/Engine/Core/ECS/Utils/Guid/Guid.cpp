#include "Guid.hpp"

uint32_t generateRandomGuid()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<uint32_t> dis;

    return dis(gen);
}