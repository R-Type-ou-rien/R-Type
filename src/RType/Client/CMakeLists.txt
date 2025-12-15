cmake_minimum_required(VERSION 3.10)

project(RTypeClient)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(PortAudio REQUIRED)
find_package(Opus REQUIRED)

include_directories(
    ../../../
)

file(GLOB_RECURSE SOURCES "*.cpp")
list(APPEND SOURCES "../../../Network/VoiceChat/VoiceChat.cpp")
list(APPEND SOURCES "../../../Network/ClientRType/ClientRType.cpp")

add_executable(r-type-client ${SOURCES})

target_link_libraries(r-type-client PRIVATE Engine RTypeCommon PortAudio::PortAudio Opus::opus)
