#!/bin/bash
scriptDirectory=$(dirname $0)
sh RemoveAutoFiles.sh
D:/OksEngine/Build/Tools/ECSGenerator2/Debug/ECSGenerator2.exe -workDir D:/Downloads/Desktop/ECSGenerator/Sources/ -cfg D:/OksEngine/Sources/Engine/Sources/ecs_gen_config.lua -includeDir D:/Downloads/Desktop/ECSGenerator/Sources/

sh GenerateBuildFiles.sh
read -p "Press enter to exit."
