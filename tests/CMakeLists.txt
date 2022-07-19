include_directories(${CMAKE_BINARY_DIR}/generated/)

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
)
target_link_libraries(
        pc_manager_test
        GTest::gtest_main
        paho-mqtt3c-static
        json-c
        libcurl
)

include(GoogleTest)
gtest_discover_tests(pc_manager_test)