// FIX: Реализация Engine::Engine и Engine::Update находится в
// ecs_generator3.cpp. Этот файл — шаблон для генерации проекта,
// он не должен компилироваться в текущей сборке (исключён в
// CMakeLists.txt через list(FILTER ... EXCLUDE REGEX "/CPPTemplates/")).
//
// Если вы хотите использовать Project.cpp как самостоятельную
// единицу трансляции — раскомментируйте реализацию ниже и
// удалите ecs_generator3.cpp, либо наоборот.

#include <CPPTemplates/Project.hpp>

