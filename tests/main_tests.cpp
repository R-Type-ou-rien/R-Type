#include <gtest/gtest.h>

#include <iostream>

int main(int argc, char** argv) {
    std::cout << "Start units tests" << std::endl;

    ::testing::InitGoogleTest(&argc, argv);

    int ret = RUN_ALL_TESTS();

    return ret;
}
