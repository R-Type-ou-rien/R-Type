cmake_minimum_required(VERSION 3.15)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake
        CACHE STRING "Vcpkg toolchain file")

cmake_minimum_required(VERSION 3.17)

set(VCPKG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg")

if(WIN32)
    set(VCPKG_EXEC "${VCPKG_ROOT}/vcpkg.exe")
    set(VCPKG_BOOTSTRAP "${VCPKG_ROOT}/bootstrap-vcpkg.bat")
else()
    set(VCPKG_EXEC "${VCPKG_ROOT}/vcpkg")
    set(VCPKG_BOOTSTRAP "${VCPKG_ROOT}/bootstrap-vcpkg.sh")
endif()

if(NOT EXISTS "${VCPKG_EXEC}")
    message(STATUS "Vcpkg non trouvé. Lancement du bootstrap automatique...")

    if(NOT EXISTS "${VCPKG_BOOTSTRAP}")
        message(FATAL_ERROR "Le script bootstrap est introuvable dans '${VCPKG_ROOT}'. As-tu fait : git submodule update --init --recursive ?")
    endif()

    execute_process(COMMAND "${VCPKG_BOOTSTRAP}" WORKING_DIRECTORY "${VCPKG_ROOT}" RESULT_VARIABLE RES)
    if(NOT RES EQUAL 0)
        message(FATAL_ERROR "Erreur lors du bootstrap de vcpkg.")
    endif()
endif()

set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain" FORCE)


project(RTypeProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_SERVER "Build for server instead of client" OFF)

add_subdirectory(src)
add_subdirectory(Network)

option(BUILD_TESTS "Build unit tests" OFF)

if(BUILD_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)
    add_subdirectory(tests)
    message(STATUS "Unit tests enabled")
endif()