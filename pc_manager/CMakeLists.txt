set(PAHO_BUILD_STATIC TRUE CACHE INTERNAL "")
add_subdirectory(external/paho.mqtt.c EXCLUDE_FROM_ALL)
include_directories(external/paho.mqtt.c/src)

add_subdirectory(external/json-c EXCLUDE_FROM_ALL)
include_directories(${CMAKE_BINARY_DIR}/external/json-c/)
include_directories(external/json-c)

add_executable(pc_manager
        src/main.c
        src/conn.c
        src/config.c
        src/tasks/shutdown.c
        src/tasks/reboot.c
        src/sensors/time.c)
target_link_libraries(pc_manager paho-mqtt3c-static json-c bsd)

install(TARGETS pc_manager)

if (WIN32)
    SET(CPACK_GENERATOR "WIX")
elseif (UNIX)
    SET(CPACK_GENERATOR "DEB")
endif ()
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "cc")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Monitor PC")
SET(CPACK_PACKAGE_VENDOR "cc")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

INCLUDE(CPack)
