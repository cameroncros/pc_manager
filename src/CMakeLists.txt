configure_file(inc/version.h.in ${CMAKE_BINARY_DIR}/generated/inc/version.h @ONLY)
include_directories(${CMAKE_BINARY_DIR}/generated/inc/)
include_directories(inc)

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
        sensors/version.c
        sensors/update.c)
target_link_libraries(
        pc_manager
        paho-mqtt3c-static
        json-c
        ssl
        crypto
        curl)

if (WIN32)
    install(TARGETS pc_manager)
elseif (UNIX)
    install(TARGETS pc_manager DESTINATION /usr/local/bin)
endif ()
