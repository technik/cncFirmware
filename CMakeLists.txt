# Software in the loop simulation for the cnc firmware
cmake_minimum_required(VERSION 3.15)
project(cncSITL)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 20)

enable_testing()

add_subdirectory(sitl)
add_subdirectory(test)