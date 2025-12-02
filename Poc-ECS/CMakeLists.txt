cmake_minimum_required(VERSION 3.11)
project(R-Type)

set(CMAKE_CXX_STANDARD 17)

find_package(SFML 2.6 COMPONENTS graphics window system REQUIRED)

include_directories(include)

add_executable(r-type 
    src/main.cpp
)

target_link_libraries(r-type PRIVATE sfml-graphics sfml-window sfml-system)