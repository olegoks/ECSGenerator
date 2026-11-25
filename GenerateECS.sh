#!/bin/bash
set -e
scriptDirectory=$(dirname "$0")
sh "$scriptDirectory/RemoveAutoFiles.sh"
D:/OksEngine/Build/Tools/ECSGenerator2/Debug/ECSGenerator2.exe \
    -workDir D:/Downloads/Desktop/ECSGenerator/Sources/ \
    -cfg D:/OksEngine/Sources/Engine/Sources/ecs_gen_config.lua \
    -includeDir D:/Downloads/Desktop/ECSGenerator/Sources/

# FIX: файл называется GenerateECSBuildFiles.sh, не GenerateBuildFiles.sh
sh "$scriptDirectory/GenerateECSBuildFiles.sh"

# FIX: не блокируем CI
if [ -t 0 ]; then
    read -p "Press enter to exit."
fi
