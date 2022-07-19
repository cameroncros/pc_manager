configure_file(sensors/version.h.in ${CMAKE_BINARY_DIR}/generated/sensors/version.h @ONLY)
include_directories(${CMAKE_BINARY_DIR}/generated/)

add_executable(pc_manager
        main.c
        utils.c
        loop.c
        conn.c
        config.c
        tasks/shutdown.c
        tasks/reboot.c
        tasks/update.c
        sensors/time.c
        sensors/version.c)
target_link_libraries(
        pc_manager
        paho-mqtt3c-static
        json-c
        libcurl)

if (WIN32)
    install(TARGETS pc_manager)
elseif (UNIX)
    install(TARGETS pc_manager DESTINATION /usr/local/bin)
endif ()
