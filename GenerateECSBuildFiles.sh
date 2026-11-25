#!/bin/bash
set -e
# FIX: mkdir -p, чтобы не падать если Build уже есть
mkdir -p Build
cd Build
cmake .. -DCMAKE_TOOLCHAIN_FILE="D:/OksEngine/vcpkg/scripts/buildsystems/vcpkg.cmake" --graphviz=DependenceGraph/dependenciesGraph.dot #--parallel
if [ -t 0 ]; then
    read -p "Press enter to exit."
fi
