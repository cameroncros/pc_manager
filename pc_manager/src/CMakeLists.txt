add_executable(pc_manager
        main.c
        conn.c
        config.c
        tasks/shutdown.c
        tasks/reboot.c
        sensors/time.c)
target_link_libraries(pc_manager paho-mqtt3c-static json-c bsd)

install(TARGETS pc_manager)