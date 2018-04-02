cmake_minimum_required(VERSION 3.10)

project(libelfbutcher VERSION 0.0.1 LANGUAGES C)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/lib)
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

add_library(elfbutcher SHARED src/elf.c)

target_compile_options(elfbutcher PRIVATE -Wall)

add_subdirectory(test ${CMAKE_SOURCE_DIR}/build EXCLUDE_FROM_ALL)
