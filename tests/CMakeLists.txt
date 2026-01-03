cmake_minimum_required(VERSION 3.17)

set(TEST_SOURCES
        main_tests.cpp
        test_network_manager.cpp
)

add_executable(unit_tests ${TEST_SOURCES})

target_link_libraries(unit_tests
        PRIVATE
        NetworkLib          # Ta lib réseau
        GTest::gtest
        GTest::gtest_main
)

target_include_directories(unit_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/Network
        ${CMAKE_SOURCE_DIR}/Network/Client
        ${CMAKE_SOURCE_DIR}/Network/Client/NetworkManager
        ${CMAKE_SOURCE_DIR}/Network/NetworkInterface
)

add_test(NAME UnitTests COMMAND unit_tests)