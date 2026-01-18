#include <iostream>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")

#else
#include <sys/types.h>
#include <sys/socket.h>
#endif

#include "ServerGameEngine.hpp"
#include "NetworkEngine/NetworkEngine.hpp"
#include "../../RType/Common/Lib/GameManager/GameManager.hpp"

int main() {
    std::cout << "[SERVER] Starting R-Type Game Server..." << std::endl;

    try {
        ServerGameEngine gameEngine;
        GameManager gm;

        gameEngine.setInitFunction(
            [&gm](std::shared_ptr<Environment> env, InputManager& inputs) { gm.init(env, inputs); });

        gameEngine.setLoopFunction(
            [&gm](std::shared_ptr<Environment> env, InputManager& inputs) { gm.update(env, inputs); });

        return gameEngine.run();
    } catch (const std::exception& e) {
        std::cerr << "[SERVER] Fatal error: " << e.what() << std::endl;
        return 84;
    } catch (...) {
        std::cerr << "[SERVER] Unknown fatal error" << std::endl;
        return 84;
    }
}
