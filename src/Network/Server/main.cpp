#include <iostream>
#include "ServerGameEngine.hpp"
#include "NetworkEngine/NetworkEngine.hpp"

int main() {
    std::cout << "[SERVER] Starting R-Type Game Server..." << std::endl;

    try {
        ServerGameEngine gameEngine;
        return gameEngine.run();
    } catch (const std::exception& e) {
        std::cerr << "[SERVER] Fatal error: " << e.what() << std::endl;
        return 84;
    } catch (...) {
        std::cerr << "[SERVER] Unknown fatal error" << std::endl;
        return 84;
    }
}
