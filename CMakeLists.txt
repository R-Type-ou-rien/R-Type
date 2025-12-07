cmake_minimum_required(VERSION 3.15)

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
    execute_process(COMMAND "${VCPKG_BOOTSTRAP}" WORKING_DIRECTORY "${VCPKG_ROOT}" RESULT_VARIABLE RES)
    if(NOT RES EQUAL 0)
        message(FATAL_ERROR "Erreur lors du bootstrap de vcpkg.")
    endif()
endif()

set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain" FORCE)

project(R-Type VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SFML 3.0 COMPONENTS Graphics Window System Audio Network CONFIG REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(src)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

set(ENGINE_SOURCES ${SOURCES})
list(FILTER ENGINE_SOURCES EXCLUDE REGEX "/src/main\\.cpp$")
list(FILTER ENGINE_SOURCES EXCLUDE REGEX "main_test_collision\\.cpp$")

if(UNIX AND NOT APPLE)
    set(CMAKE_BUILD_RPATH "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-linux/lib")
    set(CMAKE_INSTALL_RPATH "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-linux/lib")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

if (NOT BUILD_TESTS)
    add_executable(R-Type ${SOURCES})

    target_link_libraries(R-Type PRIVATE
            SFML::Graphics SFML::Window SFML::System SFML::Audio SFML::Network
            asio::asio OpenGL::GL
    )
endif()

#-----UNIT TESTS CONFIG-----
option(BUILD_TESTS "Build unit tests" OFF)

if (BUILD_TESTS)
    message(STATUS "Unit tests enabled.")
    
    enable_testing()
    include(GoogleTest)
    include(FetchContent)

    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/refs/heads/main.zip
    )

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

macro(add_rtype_test TEST_NAME FILE_PATH)
        add_executable(${TEST_NAME} 
            tests/main_tests.cpp
            ${FILE_PATH}
            ${ENGINE_SOURCES}
        )

        target_link_libraries(${TEST_NAME} PRIVATE
            GTest::gtest
            SFML::Graphics SFML::Window SFML::System SFML::Audio SFML::Network
            asio::asio OpenGL::GL
        )

        gtest_discover_tests(${TEST_NAME}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
    endmacro()

    add_rtype_test(test_collision   tests/test_collision.cpp)
    add_rtype_test(test_health      tests/test_health.cpp)
    add_rtype_test(test_damage      tests/test_damage.cpp)

else()
    message(STATUS "unit tests disabled.")
endif()
