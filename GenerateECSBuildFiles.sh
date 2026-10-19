#!/bin/bash
mkdir Build
cd Build
cmake .. -DCMAKE_TOOLCHAIN_FILE="D:/OksEngine/vcpkg/scripts/buildsystems/vcpkg.cmake" --graphviz=DependenceGraph/dependenciesGraph.dot #--parallel
read -p "Press enter to exit."
