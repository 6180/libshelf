cmake_minimum_required(VERSION 3.10)

project(elfbutcher_test VERSION 0.0.1 LANGUAGES C)

add_executable(elfbutchertest test.c)
target_include_directories(elfbutchertest BEFORE PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(elfbutchertest PUBLIC elfbutcher)
