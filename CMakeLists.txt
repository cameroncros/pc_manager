cmake_minimum_required(VERSION 3.22)

project(pc_manager)

enable_testing()

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
FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
    FetchContent_Populate(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

set(PAHO_BUILD_STATIC TRUE CACHE INTERNAL "")
set(PAHO_ENABLE_TESTING FALSE CACHE INTERNAL "")
add_subdirectory(external/paho.mqtt.c EXCLUDE_FROM_ALL)
include_directories(external/paho.mqtt.c/src)

set(HAVE_ARC4RANDOM OFF CACHE INTERNAL "")
set(BUILD_STATIC_LIBS ON CACHE INTERNAL "")
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
add_subdirectory(external/json-c EXCLUDE_FROM_ALL)
include_directories(${CMAKE_BINARY_DIR}/external/json-c/)
include_directories(external/json-c)

if (UNIX)
    add_compile_options(-Wall)
    install(FILES install/pc_manager.service
            DESTINATION /usr/lib/systemd/system)
    configure_file(install/PKGBUILD.in PKGBUILD @ONLY)
endif()

add_subdirectory(src)
add_subdirectory(tests)

if (WIN32)
    SET(CPACK_GENERATOR "WIX")
    SET(CPACK_WIX_UPGRADE_GUID b7dfe6eb-0ce0-4de6-b1f9-c03cb5718119)
elseif (UNIX)
    SET(CPACK_GENERATOR "DEB")
    SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "cc")
endif ()
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Monitor PC")
SET(CPACK_PACKAGE_VENDOR "cc")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

INCLUDE(CPack)
