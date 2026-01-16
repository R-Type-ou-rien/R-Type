#!/bin/bash

# Format with clang-format
find src tests Network test_input -type f \( -name "*.cpp" -o -name "*.hpp" \) ! -name "*sqlite3*" -exec clang-format -i -style=file {} +

# Check with cpplint
cpplint --recursive --exclude=src/Network/Database/sqlite3.c --exclude=src/Network/Database/sqlite3.h --exclude=src/Network/* --exclude=Network/* src tests Network test_input