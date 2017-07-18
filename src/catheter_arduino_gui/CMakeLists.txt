cmake_minimum_required(VERSION 2.8)

project(catheter_arduino_gui)

find_package(Boost COMPONENTS system thread REQUIRED)
find_package(wxWidgets COMPONENTS core base adv propgrid REQUIRED )
find_package(GTest REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -O0" )

find_package(catkin REQUIRED COMPONENTS roslint)

catkin_package(
      INCLUDE_DIRS include
      LIBRARIES pc_utils_lib
      DEPENDS 
        GTest 
        Boost COMPONENTS system thread
        wxWidgets COMPONENTS core base adv propgrid
)

include_directories(
${catkin_INCLUDE_DIRS}
${BOOST_INCLUDE_DIRS}
${GTEST_INCLUDE_DIRS}
include
)


include( "${wxWidgets_USE_FILE}" )

# common folder libs

add_library(catheter_commands_lib src/com/catheter_commands.cpp)

add_library(pc_utils_lib src/com/pc_utils.cpp)

# gui folder libs


add_library(catheter_grid_lib src/gui/catheter_grid.cpp)
add_library(status_frame_lib src/gui/status_frame.cpp)
add_library(status_text_lib src/gui/status_text.cpp)

# ser folder libs

add_library(serial_sender_lib src/ser/serial_sender.cpp)
add_library(serial_thread_lib src/ser/serial_thread.cpp)
add_library(simple_serial_lib src/ser/simple_serial.cpp)


#other libs
add_library(catheter_analog_digital_libs src/hardware/digital_analog_conversions.cpp)



#final executable.

add_executable(catheter_gui src/gui/catheter_gui.cpp)

target_link_libraries(catheter_commands_lib
catheter_analog_digital_libs
)

target_link_libraries(serial_sender_lib
simple_serial_lib
catheter_analog_digital_libs
catheter_commands_lib
${Boost_LIBRARIES}
${Boost_SYSTEM_LIBRARY}
${Boost_THREAD_LIBRARY}
)

target_link_libraries(serial_thread_lib
serial_sender_lib
catheter_analog_digital_libs
catheter_commands_lib
${Boost_LIBRARIES}
${Boost_SYSTEM_LIBRARY}
${Boost_THREAD_LIBRARY}
)

target_link_libraries(catheter_grid_lib
    ${wxWidgets_ADVANCED_LIBRARIES}
)

target_link_libraries(status_frame_lib
    ${wxWidgets_ADVANCED_LIBRARIES}
)

target_link_libraries(status_text_lib
  ${wxWidgets_ADVANCED_LIBRARIES}
)

target_link_libraries(catheter_commands_lib
catheter_analog_digital_libs
)

target_link_libraries(serial_sender_lib
simple_serial_lib
catheter_analog_digital_libs
catheter_commands_lib
${Boost_LIBRARIES}
${Boost_SYSTEM_LIBRARY}
${Boost_THREAD_LIBRARY}
)

target_link_libraries(serial_thread_lib
serial_sender_lib
catheter_analog_digital_libs
catheter_commands_lib
${Boost_LIBRARIES}
${Boost_SYSTEM_LIBRARY}
${Boost_THREAD_LIBRARY}
)

target_link_libraries(catheter_grid_lib
    ${wxWidgets_ADVANCED_LIBRARIES}
)

target_link_libraries(status_frame_lib
    ${wxWidgets_ADVANCED_LIBRARIES}
)

target_link_libraries(status_text_lib
  ${wxWidgets_ADVANCED_LIBRARIES}
)

target_link_libraries(
catheter_gui
pc_utils_lib
catheter_grid_lib
status_frame_lib
status_text_lib
serial_sender_lib
serial_thread_lib
simple_serial_lib
catheter_analog_digital_libs
${Boost_LIBRARIES}
${Boost_SYSTEM_LIBRARY}
${Boost_THREAD_LIBRARY}
${wxWidgets_ADVANCED_LIBRARIES}
${wxWidgets_LIBRARIES}
)

roslint_cpp()

roslint_add_test()

# Add gtest for pc_utils
catkin_add_gtest(test_pc_utils test/test_pc_utils.cpp WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
target_link_libraries(
    test_pc_utils
    pc_utils_lib
    ${GTEST_LIBRARIES}
    pthread
)

catkin_add_gtest(test_adc_dac test/test_adc_dac.cpp WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
target_link_libraries(
    test_adc_dac
    catheter_analog_digital_libs
    ${GTEST_LIBRARIES}
    pthread
)
