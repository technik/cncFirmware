#General config for all tests
include_directories(
	../sitl
	../.pio/libdeps/megaatmega2560/etl/src)

add_compile_definitions(SITL)

# Motion controller test
add_executable(motionControllerTest motion_controller_test.cpp ../src/motionController.cpp)
set_target_properties(motionControllerTest PROPERTIES FOLDER test/)
add_test(motion_controller_test motionControllerTest)