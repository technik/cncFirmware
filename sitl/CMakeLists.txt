# Software in the loop simulation for the cnc firmware
cmake_minimum_required(VERSION 3.15)
project(cncSITL)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 20)

add_executable(cncSITL
	Arduino.cpp
	../src/AnalogJoystick.h
	../src/GCode.h
	../src/main.cpp
	../src/motionController.cpp
	../src/motionController.h
	../src/stepperDriver.h
	../src/vector.h)

target_compile_definitions(cncSITL PRIVATE SITL)
target_include_directories(cncSITL PUBLIC ${CMAKE_CUR_PROJECT_DIR}../sitl)
target_include_directories(cncSITL PUBLIC ${CMAKE_CUR_PROJECT_DIR}../src)
target_include_directories(cncSITL PUBLIC ${CMAKE_CUR_PROJECT_DIR}../.pio/libdeps/megaatmega2560/etl/src)