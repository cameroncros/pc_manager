include_directories(../src/inc)

add_executable(
        pc_manager_test
        system_test.cpp
        update.cpp
        ../src/utils.c
        ../src/loop.c
        ../src/conn.c
        ../src/config.c
        ../src/tasks/shutdown.c
        ../src/tasks/reboot.c
        ../src/tasks/update.c
        ../src/sensors/time.c
        ../src/sensors/version.c
        ../src/sensors/update.c
)
if (WIN32)
    target_link_libraries(
            pc_manager_test
            GTest::gtest_main
            paho-mqtt3c-static
            json-c
            libcurl
    )
elseif (UNIX)
    target_link_libraries(
            pc_manager_test
            GTest::gtest_main
            paho-mqtt3c-static
            json-c
            libcurl
            z
            ssl
            crypto
    )
endif ()


include(GoogleTest)
gtest_discover_tests(pc_manager_test)