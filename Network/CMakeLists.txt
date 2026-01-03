cmake_minimum_required(VERSION 3.10)

project(NetworkTests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_package(asio CONFIG REQUIRED)

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/NetworkInterface
        ${CMAKE_CURRENT_SOURCE_DIR}/Database
        ${CMAKE_CURRENT_SOURCE_DIR}/Lobby
        ${CMAKE_CURRENT_SOURCE_DIR}/Client
        ${CMAKE_CURRENT_SOURCE_DIR}/Client/NetworkManager
        ${CMAKE_CURRENT_SOURCE_DIR}/Server
)

# Common sources for Network Library
set(NETWORK_SOURCES
        NetworkInterface/Connection.hpp
        NetworkInterface/ServerInterface.hpp
        NetworkInterface/ClientInterface.hpp
        NetworkInterface/MsgQueue.hpp
        NetworkInterface/message.hpp
        Database/Database.cpp
        Database/Database.hpp
        Database/sqlite3.c
        Database/sqlite3.h
        Lobby/Lobby.hpp
        Client/Client.cpp
        Client/Client.hpp
        Client/NetworkManager/NetworkManager.cpp
        Client/NetworkManager/NetworkManager.hpp
        Server/Server.cpp
        Server/Server.hpp
)

add_library(NetworkLib STATIC ${NETWORK_SOURCES})

# PUBLIC pour que les exécutables héritent des includes d'asio
target_link_libraries(NetworkLib
        PUBLIC
        asio::asio
        PRIVATE
        Threads::Threads
        ${CMAKE_DL_LIBS}
)

# Server Executable
add_executable(r-type_server Server/main.cpp)
target_link_libraries(r-type_server PRIVATE NetworkLib)

# Client Test Executable
add_executable(r-type_client_test Client/main.cpp)
target_link_libraries(r-type_client_test PRIVATE NetworkLib)