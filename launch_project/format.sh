#!/bin/bash

find src tests Network test_input -type f \( -name "*.cpp" -o -name "*.hpp" \) -exec clang-format -i -style=file {} +

