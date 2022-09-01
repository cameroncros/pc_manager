include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
    FetchContent_Populate(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

include_directories(../src/inc)

add_executable(
        pc_manager_test
        system_test.cpp
        update_test.cpp
        config_test.cpp
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