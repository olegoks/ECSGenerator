cmake_minimum_required(VERSION 3.26.4)
project(Generator)

set(CMAKE_RULE_MESSAGES ON)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_COMPILER "clang++")

# set(CMAKE_TOOLCHAIN_FILE "D:/OksEngine/vcpkg/scripts/buildsystems/vcpkg.cmake") #For vcpkg

add_executable(ecs_generator3)

add_subdirectory(Sources)

include(auto_Help.cmake)

# Get all sources
include(auto_Modules.cmake)

GetModuleSources(MODULE_SOURCES)

# get_source_files(
#     PROJECT_SOURCES
#     "${CMAKE_CURRENT_LIST_DIR}/Sources"
#     "${CMAKE_CURRENT_LIST_DIR}/Packages"
# )

get_source_files(
    PROJECT_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/Sources"
)

list(APPEND ALL_SOURCES ${MODULE_SOURCES} ${PROJECT_SOURCES})
list(REMOVE_DUPLICATES ALL_SOURCES)
list(FILTER PROJECT_SOURCES EXCLUDE REGEX "/CPPTemplates/")
list(FILTER PROJECT_SOURCES EXCLUDE REGEX "/CMakeTemplates/")
#

set(CMAKE_SUPPRESS_REGENERATION true) # To exclude ZERO_CHECK target
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakeTargets")

#LLVM CLANG
# set(LLVM_DIR "D:/OksEngine/vcpkg/installed/x64-windows/share/llvm")
# set(Clang_DIR "D:/OksEngine/vcpkg/installed/x64-windows/share/clang")

# function(get_source_files FILE_LIST)
#     set(TEMP_FILES "")
#     foreach(DIR ${ARGN})
#         file(GLOB_RECURSE DIR_FILES "${DIR}/*.cpp" "${DIR}/*.hpp")
#         list(APPEND TEMP_FILES ${DIR_FILES})
#     endforeach()
#     #message(STATUS "Total files: ${TEMP_FILES}")
#     set(${FILE_LIST} ${TEMP_FILES} PARENT_SCOPE)
# endfunction()

# # Функция для создания source_group для в соответствии с их местоположением относительно корневой директории
# function(organize_source_files DIRECTORY FILES)
#     foreach(FILE_PATH IN LISTS FILES)
#         # Убираем корневую часть пути к файлу
#         file(RELATIVE_PATH REL_PATH "${DIRECTORY}" ${FILE_PATH})
#         # Извлекаем родительский путь
#         get_filename_component(PARENT_DIR "${REL_PATH}" DIRECTORY)
#         # Заменяем '/' на '\\' в пути, для правильной работы source_group
#         string(REPLACE "/" "\\" GROUP_NAME "Sources/${PARENT_DIR}")
#         # Создаем source_group
#         source_group("${GROUP_NAME}" FILES "${FILE_PATH}")
#         #message(STATUS "${GROUP_NAME}=${${GROUP_NAME}} ${FILE_PATH}=${${FILE_PATH}} ")
#     endforeach()
# endfunction()




# Организуем файлы по группам
organize_source_files(${CMAKE_CURRENT_LIST_DIR} "${PROJECT_SOURCES}")


#Generate cpp includes with cmake files

function(GenerateTemplates TEMPLATE_PATH DEST_PATH)
    # Пропускаем первые два аргумента (TEMPLATE_PATH и DEST_PATH)
    set(INDEX 2)
    while(INDEX LESS ${ARGC})
        math(EXPR NAME_INDEX "${INDEX}")
        math(EXPR PATH_INDEX "${INDEX} + 1")

        list(GET ARGV ${NAME_INDEX} VAR_NAME)
        list(GET ARGV ${PATH_INDEX} FILE_PATH)

        message(STATUS "Processing: ${VAR_NAME} = ${FILE_PATH}")

        if(EXISTS "${FILE_PATH}")
            file(READ "${FILE_PATH}" CONTENT)
            set(${VAR_NAME} "${CONTENT}")
        else()
            message(WARNING "File not found: ${FILE_PATH} — using empty content")
            set(${VAR_NAME} "")
        endif()

        math(EXPR INDEX "${INDEX} + 2")
    endwhile()

    # Создаём выходную директорию, если её нет
    get_filename_component(DEST_DIR "${DEST_PATH}" DIRECTORY)
    file(MAKE_DIRECTORY "${DEST_DIR}")

    message(STATUS "Configure file: ${DEST_PATH}")
    configure_file(
        "${TEMPLATE_PATH}"
        "${DEST_PATH}"
        @ONLY
    )

endfunction()

GenerateTemplates(
    "${CMAKE_CURRENT_SOURCE_DIR}/CMakeTemplates/Help.h.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/Sources/CMakeTemplates_Generated/Help.h"
    "CONTENT" "${CMAKE_CURRENT_SOURCE_DIR}/CMakeTemplates/Help.cmake"
)

GenerateTemplates(
    "${CMAKE_CURRENT_SOURCE_DIR}/CPPTemplates/CPP.hpp.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/Sources/CPPTemplates_Generated/auto_CPP.hpp"
    "MAIN_CPP_CONTENT" "${CMAKE_CURRENT_SOURCE_DIR}/CPPTemplates/main.cpp"
    "PROJECT_HPP_CONTENT" "${CMAKE_CURRENT_SOURCE_DIR}/CPPTemplates/Project.hpp"
    "PROJECT_CPP_CONTENT" "${CMAKE_CURRENT_SOURCE_DIR}/CPPTemplates/Project.cpp"
)

#DEBUG COMMANDS
if(CMAKE_BUILD_TYPE_INIT STREQUAL "Debug")
    set_target_properties(ecs_generator3
        PROPERTIES
        VS_DEBUGGER_COMMAND_ARGUMENTS
        "$<$<CONFIG:Debug>: -cfg ${CMAKE_CURRENT_LIST_DIR}/../config.lua -scn resource:/Scenes/OksEngine.EmptySceneDebug.scn>$<$<CONFIG:Release>: -cfg ${CMAKE_CURRENT_LIST_DIR}/../config.lua -scn ${CMAKE_CURRENT_LIST_DIR}/../Resources/Scenes/OksEngine.EmptyScene.scn>$<$<CONFIG:RelWithDebInfo>: -cfg ${CMAKE_CURRENT_LIST_DIR}/../config.lua -scn resource:/Scenes/OksEngine.EmptyScene.scn>"
    )
endif()

target_compile_options(ecs_generator3 PUBLIC "/MP" "/bigobj")

if(WIN32)
    set(PKG_CONFIG_EXECUTABLE "D:/OksEngine/vcpkg/installed/x64-windows/tools/pkgconf/pkgconf.exe"
        CACHE FILEPATH "pkg-config executable")

    if(NOT EXISTS "${PKG_CONFIG_EXECUTABLE}")
        message(FATAL_ERROR "pkgconf.exe not found at: ${PKG_CONFIG_EXECUTABLE}")
    endif()

    set(ENV{PKG_CONFIG_PATH} "D:/OksEngine/vcpkg/installed/x64-windows/lib/pkgconfig")
endif()

#GRAPHVIZ
find_package(PkgConfig REQUIRED)
target_compile_options(ecs_generator3 PUBLIC "/MP" "/bigobj")
pkg_check_modules(GRAPHVIZ REQUIRED libgvc)
message(STATUS "${GRAPHVIZ_INCLUDE_DIRS}")
target_include_directories(
    ecs_generator3
    PUBLIC
    ${GRAPHVIZ_INCLUDE_DIRS}
    "${CMAKE_BINARY_DIR}/CMakeTemplates_generated")

message(STATUS "${GRAPHVIZ_LIBRARY_DIRS}")
target_link_directories(ecs_generator3 PUBLIC ${GRAPHVIZ_LIBRARY_DIRS})



find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

llvm_map_components_to_libnames(LLVM_LIBS core irreader support)

set(CLANG_LIBS
    clangFormat
    clangToolingCore
    clangBasic
    clangLex
    clangSerialization
    clangRewrite
    clangAST
    clangSupport
    clangFrontend
    clangDriver
    LLVMSupport
)

set_target_properties(ecs_generator3
    PROPERTIES
    VS_DEBUGGER_COMMAND_ARGUMENTS
    "$<$<CONFIG:Debug>: -ecs_project_file D:/Downloads/Desktop/ECSGenerator/ecs_generator3.ecs_project>$<$<CONFIG:Release>: -ecs_project_file D:/Downloads/Desktop/ECSGenerator/ecs_generator3.ecs_project>$<$<CONFIG:RelWithDebInfo>: -ecs_project_file D:/Downloads/Desktop/ECSGenerator/ecs_generator3.ecs_project>"
)

target_include_directories(ecs_generator3
    PUBLIC
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}
    ${CMAKE_CURRENT_LIST_DIR}/Sources/)

target_sources(ecs_generator3
    PUBLIC
    ${ALL_SOURCES}

    PRIVATE
)


include(auto_IncludeDirectories.cmake)
target_include_directories(ecs_generator3
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR})

if(NOT PKG_CONFIG_EXECUTABLE)
    find_program(PKG_CONFIG_EXECUTABLE NAMES pkgconf pkg-config
        HINTS "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/pkgconf")
endif()


target_link_libraries(ecs_generator3
    PUBLIC
    ${GRAPHVIZ_LIBRARIES}
    ${CLANG_LIBS}
    ${LLVM_LIBS}
)

target_include_directories(ecs_generator3 PUBLIC ${CMAKE_CURRENT_LIST_DIR})

#lib3dpart
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(ecs_generator3 PRIVATE glfw)
find_package(Lua REQUIRED)
target_include_directories(ecs_generator3 PUBLIC ${LUA_INCLUDE_DIR})
target_link_libraries(ecs_generator3 PRIVATE ${LUA_LIBRARIES})
find_package(magic_enum CONFIG REQUIRED)

find_package(winpixevent CONFIG REQUIRED)
target_link_libraries(ecs_generator3 PUBLIC 
Microsoft::WinPixEventRuntime
magic_enum::magic_enum)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(ecs_generator3 PRIVATE 
    fmt::fmt         # Явно указываем fmt, так как spdlog зависит от него
)


message("ZLIB_FOUND: ${ZLIB_FOUND}")
message("ZLIB_INCLUDE_DIRS: ${ZLIB_INCLUDE_DIRS}")
message("ZLIB_LIBRARIES: ${ZLIB_LIBRARIES}")
message("ZLIB_LIBRARY: ${ZLIB_LIBRARY}")
message("ZLIB_LIBRARY: ${ZLIB_LIBRARY_RELEASE}")
message("ZLIB_LIBRARY_DEBUG: ${ZLIB_LIBRARY_DEBUG}")

add_custom_command(TARGET ecs_generator3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${ZLIB_LIBRARY_RELEASE}"
    "${ZLIB_LIBRARY_DEBUG}"
    "${GRAPHVIZ_PREFIX}/bin/cdt.dll"
    "${GRAPHVIZ_PREFIX}/bin/cgraph.dll"
    "${GRAPHVIZ_PREFIX}/bin/config6"
    "${GRAPHVIZ_PREFIX}/bin/gvc.dll"
    "${GRAPHVIZ_PREFIX}/bin/gvplugin_core.dll"
    "${GRAPHVIZ_PREFIX}/bin/gvplugin_dot_layout.dll"
    "${GRAPHVIZ_PREFIX}/bin/gvplugin_gd.dll"
    "${GRAPHVIZ_PREFIX}/bin/gvplugin_gdiplus.dll"
    "${GRAPHVIZ_PREFIX}/bin/gvplugin_kitty.dll"
    "${GRAPHVIZ_PREFIX}/bin/gvplugin_neato_layout.dll"
    "${GRAPHVIZ_PREFIX}/bin/gvplugin_pango.dll"
    "${GRAPHVIZ_PREFIX}/bin/gvplugin_rsvg.dll "
    "${GRAPHVIZ_PREFIX}/bin/gvplugin_vt.dll"
    "${GRAPHVIZ_PREFIX}/bin/gvplugin_webp.dll"
    "${GRAPHVIZ_PREFIX}/bin/pathplan.dll"
    "${GRAPHVIZ_PREFIX}/bin/xdot.dll"
    $<TARGET_FILE_DIR:ecs_generator3>)

