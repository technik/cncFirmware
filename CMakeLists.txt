# Software in the loop simulation for the cnc firmware
cmake_minimum_required(VERSION 3.15)
project(cncSITL)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 20)
add_compile_definitions(NO_MIN_MAX)

enable_testing()

add_subdirectory(sitl)
add_subdirectory(test)