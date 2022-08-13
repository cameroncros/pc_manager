cmake_minimum_required(VERSION 3.22)

project(pc_manager VERSION 1.6.0)

enable_testing()

SET(CMAKE_BUILD_TYPE Release)

configure_file(src/inc/version.h.in ${CMAKE_BINARY_DIR}/generated/inc/version.h @ONLY)
include_directories(${CMAKE_BINARY_DIR}/generated/inc/)

execute_process(
        COMMAND git log -1 --format=%h
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE GIT_HASH_SHORT
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
        COMMAND git log -1 --format=%H
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
        COMMAND git log -1 --format=%H
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/external/paho.mqtt.c
        OUTPUT_VARIABLE GIT_HASH_PAHO
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
        COMMAND git log -1 --format=%H
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/external/json-c
        OUTPUT_VARIABLE GIT_HASH_JSONC
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
        COMMAND git log -1 --format=%H
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/external/curl
        OUTPUT_VARIABLE GIT_HASH_CURL
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

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


set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
set(LINK_SEARCH_START_STATIC TRUE)
set(LINK_SEARCH_END_STATIC TRUE)

set(SHARED_LIBS OFF)
set(STATIC_LIBS ON)

set(PAHO_BUILD_STATIC TRUE CACHE INTERNAL "")
set(PAHO_ENABLE_TESTING FALSE CACHE INTERNAL "")
set(USE_LIBIDN2 FALSE CACHE INTERNAL "")
add_subdirectory(external/paho.mqtt.c EXCLUDE_FROM_ALL)
include_directories(external/paho.mqtt.c/src)

set(HAVE_ARC4RANDOM OFF CACHE INTERNAL "")
set(BUILD_STATIC_LIBS ON CACHE INTERNAL "")
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
set(DISABLE_EXTRA_LIBS ON CACHE INTERNAL "")
add_subdirectory(external/json-c EXCLUDE_FROM_ALL)
include_directories(${CMAKE_BINARY_DIR}/external/json-c/)
include_directories(external/json-c)

if (WIN32)
    set(ssize_t __int64 CACHE INTERNAL "")

    set(CURL_USE_SCHANNEL true CACHE INTERNAL "")
    set(HTTP_ONLY true CACHE INTERNAL "")
    set(CURL_STATICLIB true CACHE INTERNAL "")
    set(CURL_ZLIB OFF CACHE INTERNAL "")
    set(BUILD_CURL_EXE OFF CACHE INTERNAL "")
    include_directories(external/curl/include)
    add_subdirectory(external/curl EXCLUDE_FROM_ALL)
    link_directories(${CMAKE_BINARY_DIR}/external/curl/lib)
    unset(ssize_t)
elseif (UNIX)
    add_definitions(-DCURL_STATICLIB -DCURL_USE_OPENSSL)
    set(CURL_USE_OPENSSL true CACHE INTERNAL "")
    set(HTTP_ONLY true CACHE INTERNAL "")
    set(CURL_STATICLIB true CACHE INTERNAL "")
    set(BUILD_CURL_EXE OFF CACHE INTERNAL "")
    add_subdirectory(external/curl EXCLUDE_FROM_ALL)
    include_directories(external/curl/include)
    link_directories(${CMAKE_BINARY_DIR}/external/curl/lib)
endif ()


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
    SET(CPACK_WIX_PATCH_FILE ${CMAKE_CURRENT_SOURCE_DIR}/install/windows_service.xml)
elseif (UNIX)
    SET(CPACK_GENERATOR "DEB")
    SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "cc")
endif ()
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Monitor PC")
SET(CPACK_PACKAGE_VENDOR "cc")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

INCLUDE(CPack)
