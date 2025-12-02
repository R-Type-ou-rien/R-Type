cmake_minimum_required(VERSION 3.11)
project(R-Type)

set(CMAKE_CXX_STANDARD 17)

find_package(SFML 2.6 COMPONENTS graphics window system REQUIRED)

# Ajoute tous les .hpp/.cpp de src/ au include

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

include_directories(src)

# Liste explicitement les sources (clair et propre)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

# Exclude test files with their own main functions
list(FILTER SOURCES EXCLUDE REGEX ".*test\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*velocity/main\\.cpp$")

add_executable(r-type ${SOURCES})

target_link_libraries(r-type PRIVATE sfml-graphics sfml-window sfml-system)


