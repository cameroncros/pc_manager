cmake_minimum_required(VERSION 3.22)

project(pc_manager)

SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_PACKAGE_VERSION_PATCH "1")

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(PAHO_BUILD_STATIC TRUE CACHE INTERNAL "")
add_subdirectory(external/paho.mqtt.c EXCLUDE_FROM_ALL)
include_directories(external/paho.mqtt.c/src)

set(HAVE_ARC4RANDOM OFF CACHE INTERNAL "")
add_subdirectory(external/json-c EXCLUDE_FROM_ALL)
include_directories(${CMAKE_BINARY_DIR}/external/json-c/)
include_directories(external/json-c)

add_subdirectory(src)
add_subdirectory(tests)

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
