cmake_minimum_required(VERSION 3.5)

project(libshelf_project VERSION 0.0.1 LANGUAGES C)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/lib)
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

set(LIBSHELF_SOURCES
    src/shelf.c
    src/shelf_dump.c
    src/section.c
    src/symbol.c
    src/shelf_profiler.c
)

add_library(libshelf SHARED ${LIBSHELF_SOURCES})
set_target_properties(libshelf PROPERTIES OUTPUT_NAME "shelf")
target_compile_options(libshelf PRIVATE -std=gnu11 -Wall -Wextra -O3)
target_include_directories(libshelf PRIVATE include src)

install(TARGETS libshelf DESTINATION /usr/lib/shelf)
install(DIRECTORY include/ DESTINATION /usr/include/shelf
    FILES_MATCHING PATTERN "*.h"
)

add_library(debug SHARED ${LIBSHELF_SOURCES})
set_target_properties(debug PROPERTIES OUTPUT_NAME "shelf")
target_compile_options(debug PRIVATE -std=c11 -Wall -Wextra -Og -g)
target_include_directories(debug PRIVATE include src)

# Test program project.
add_subdirectory(test ${CMAKE_SOURCE_DIR}/build EXCLUDE_FROM_ALL)
