cmake_minimum_required(VERSION 3.22)
project(pc_manager)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(external/paho.mqtt.c)

add_executable(pc_manager main.c)
target_link_libraries(pc_manager paho-mqtt3c)
