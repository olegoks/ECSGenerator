cmake_minimum_required(VERSION  3.26.4)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


add_library(Datastructures STATIC)
set(SourcesDir "")

file(GLOB_RECURSE PUBLIC_SOURCES
    RELATIVE ${CMAKE_CURRENT_LIST_DIR}
    LIST_DIRECTORIES false
    *.hpp
    *.h
)

file(GLOB_RECURSE
 PRIVATE_SOURCES
    RELATIVE ${CMAKE_CURRENT_LIST_DIR}
    LIST_DIRECTORIES false
    *.cpp
    *.c
)

target_sources(Datastructures
    PUBLIC
        ${PUBLIC_SOURCES}
        ${PRIVATE_SOURCES}
    )

        
source_group("Sources"
FILES
${PUBLIC_SOURCES}
${PRIVATE_SOURCES})

target_include_directories(Datastructures
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        )

target_link_libraries(Datastructures
    PUBLIC
        OS
        )
