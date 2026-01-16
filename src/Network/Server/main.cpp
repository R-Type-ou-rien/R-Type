#include "Server.hpp"
#include <iostream>
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <iphlpapi.h>
    #pragma comment(lib, "Ws2_32.lib")
    #pragma comment(lib, "Iphlpapi.lib")

#else
    #include <ifaddrs.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <sys/types.h>
    #include <sys/socket.h>
#endif

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
