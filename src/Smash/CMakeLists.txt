cmake_minimum_required(VERSION 3.10)

project(SmashClient)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    ../../../
)

file(GLOB_RECURSE SOURCES "*.cpp")

add_executable(smash-client ${SOURCES})

target_link_libraries(smash-client PRIVATE Engine)