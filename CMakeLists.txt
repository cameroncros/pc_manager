cmake_minimum_required(VERSION 3.22)
project(pc_manager)

set(PAHO_BUILD_STATIC TRUE CACHE INTERNAL "")
add_subdirectory(external/paho.mqtt.c)

add_executable(pc_manager main.c conn.c utils.h)
target_link_libraries(pc_manager paho-mqtt3c-static)
