#!/bin/bash
echo "Formattage du code..."
find src tests -type f \( -name "*.cpp" -o -name "*.hpp" \) \
    ! -path "*/build/*" \
    ! -path "*/vcpkg/*" \
    -exec clang-format -i -style=file {} +
echo "Terminé."
