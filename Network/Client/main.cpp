#include "Client.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <vector>
#include <cstring>

using namespace network;

void receive_loop(network::Client& client) {
    while (true) {
        if (!client.Incoming().empty()) {
            auto msg = client.ReadIncomingMessage();
            if (msg.id == GameEvents::NONE)
                continue;
            std::cout << "[CLIENT] Message Received with ID: " << (int)msg.id << "\n";

            switch (msg.id) {
                case GameEvents::S_REGISTER_OK:
                    std::cout << "[SERVER] Registration Successful! Token received.\n";
                    break;
                case GameEvents::S_LOGIN_OK:
                    std::cout << "[SERVER] Login Successful!\n";
                    break;
                case GameEvents::S_LOGIN_KO:
                    std::cout << "[SERVER] Login Failed.\n";
                    break;
                case GameEvents::S_ROOMS_LIST: {
                    std::cout << "[SERVER] Room List Received:\n";
                    uint32_t nb = 0;
                    msg.msg >> nb;

                    std::vector<lobby_info> receivedLobbies;
                    for (uint32_t i = 0; i < nb; ++i) {
                        lobby_info info;
                        msg.msg >> info;
                        receivedLobbies.push_back(info);
                    }

                    if (nb == 0) {
                        std::cout << "  No lobbies found.\n";
                    } else {
                        for (int i = receivedLobbies.size() - 1; i >= 0; --i) {
                            const auto& info = receivedLobbies[i];
                            std::string stateStr =
                                (info.state == 0) ? "WAITING" : ((info.state == 1) ? "IN_GAME" : "FINISHED");
                            std::cout << "  [" << info.id << "] " << info.name << " (" << info.nbConnectedPlayers << "/"
                                      << info.maxPlayers << ") "
                                      << "- " << stateStr << "\n";
                        }
                    }
                    break;
                }
                case GameEvents::S_ROOM_JOINED:
                    std::cout << "[SERVER] Joined Room!\n";
                    break;
                case GameEvents::S_ROOM_NOT_JOINED:
                    std::cout << "[SERVER] Failed to join room.\n";
                    break;
                case GameEvents::S_PLAYER_JOINED:
                    std::cout << "[SERVER] A player joined the lobby.\n";
                    break;
                case GameEvents::S_PLAYER_LEAVE:
                    std::cout << "[SERVER] A player left the lobby.\n";
                    break;
                default:
                    break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int main(int argc, char* argv[]) {
    std::string host = "127.0.0.1";
    uint16_t port = 4040;

    if (argc >= 2) {
        host = argv[1];
    }
    if (argc >= 3) {
        port = std::stoi(argv[2]);
    }

    network::Client client;
    std::cout << "[CLIENT] Connecting to " << host << ":" << port << "...\n";

    if (!client.Connect(host, port)) {
        std::cerr << "[CLIENT] Connection failed.\n";
        return 1;
    }
    std::cout << "[CLIENT] Connected!\n";

    // Start receiver thread
    std::thread receiver_thread(receive_loop, std::ref(client));
    receiver_thread.detach();

    std::cout << "Available commands:\n";
    std::cout << "  register <username> <password>\n";
    std::cout << "  login <username> <password>\n";
    std::cout << "  create <lobby_name>\n";
    std::cout << "  list\n";
    std::cout << "  join <lobby_id>\n";
    std::cout << "  leave\n";
    std::cout << "  exit\n";

    std::string line;
    while (std::getline(std::cin, line)) {
        std::stringstream ss(line);
        std::string command;
        ss >> command;

        if (command == "exit") {
            break;
        } else if (command == "register") {
            std::string user, pass;
            if (ss >> user >> pass) {
                client.RegisterServer(user, pass);
                std::cout << "[CMD] Register sent.\n";
            } else {
                std::cout << "Usage: register <username> <password>\n";
            }
        } else if (command == "login") {
            std::string user, pass;
            if (ss >> user >> pass) {
                client.LoginServer(user, pass);
                std::cout << "[CMD] Login sent.\n";
            } else {
                std::cout << "Usage: login <username> <password>\n";
            }
        } else if (command == "create") {
            std::string lobbyName;
            if (ss >> lobbyName) {
                char name[32] = {0};
                std::strncpy(name, lobbyName.c_str(), 32);
                client.AddMessageToServer(GameEvents::C_NEW_LOBBY, 0, name);
                std::cout << "[CMD] Create Lobby sent.\n";
            } else {
                std::cout << "Usage: create <lobby_name>\n";
            }
        } else if (command == "list") {
            client.AddMessageToServer(GameEvents::C_LIST_ROOMS, 0, NULL);
            std::cout << "[CMD] List Rooms sent.\n";
        } else if (command == "join") {
            uint32_t id;
            if (ss >> id) {
                client.AddMessageToServer(GameEvents::C_JOIN_ROOM, 0, id);
                std::cout << "[CMD] Join Room sent.\n";
            } else {
                std::cout << "Usage: join <lobby_id>\n";
            }
        } else if (command == "leave") {
            client.AddMessageToServer(GameEvents::C_ROOM_LEAVE, 0, NULL);
            std::cout << "[CMD] Leave Room sent.\n";
        }
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));

    client.Disconnect();
    return 0;
}
