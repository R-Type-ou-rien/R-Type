#include "Server.hpp"
#include <iostream>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main() {
    std::cout << std::unitbuf;
    network::Server server(4040, 30);

    if (server.Start()) {
        std::cout << "[SERVER] Server running. Press Ctrl+C to stop.\n";

        while (1) {
            server.Update(-1, true);
        }
    } else {
        std::cerr << "[SERVER] Failed to start server.\n";
        return 1;
    }

    return 0;
}
