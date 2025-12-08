cmake_minimum_required(VERSION 3.16)
project(test_input LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# On veut utiliser les libs statiques de SFML (c'est ce que tu as installé)
set(SFML_STATIC_LIBRARIES ON)

# SFML 3 est installée dans /usr/local/lib64/cmake/SFML
set(SFML_DIR "/usr/local/lib64/cmake/SFML")

find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)

add_executable(test_input
    main_test_input.cpp
    ../src/ecs/Input/InputManager.cpp
)

# Inclure le dossier où sont tes headers d'input
target_include_directories(test_input PRIVATE
    ../src/ecs/Input
)

# 🔑 C'est ÇA qui corrige l'édition de liens :
# on link les targets modernes SFML 3
target_link_libraries(test_input PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
)
