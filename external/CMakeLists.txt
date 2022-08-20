set(SHARED_LIBS OFF)
set(STATIC_LIBS ON)

set(PAHO_BUILD_STATIC TRUE CACHE INTERNAL "")
set(PAHO_ENABLE_TESTING FALSE CACHE INTERNAL "")
set(USE_LIBIDN2 FALSE CACHE INTERNAL "")
add_subdirectory(paho.mqtt.c EXCLUDE_FROM_ALL)
include_directories(paho.mqtt.c/src)

set(HAVE_ARC4RANDOM OFF CACHE INTERNAL "")
set(BUILD_STATIC_LIBS ON CACHE INTERNAL "")
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
set(DISABLE_EXTRA_LIBS ON CACHE INTERNAL "")
add_subdirectory(json-c EXCLUDE_FROM_ALL)
include_directories(${CMAKE_BINARY_DIR}/json-c/)
include_directories(json-c)

if (WIN32)
    set(ssize_t __int64 CACHE INTERNAL "")

    set(CURL_USE_SCHANNEL true CACHE INTERNAL "")
    set(HTTP_ONLY true CACHE INTERNAL "")
    set(CURL_STATICLIB true CACHE INTERNAL "")
    set(CURL_ZLIB OFF CACHE INTERNAL "")
    set(BUILD_CURL_EXE OFF CACHE INTERNAL "")
    include_directories(curl/include)
    add_subdirectory(curl EXCLUDE_FROM_ALL)
    link_directories(${CMAKE_BINARY_DIR}/curl/lib)
    unset(ssize_t)
elseif (UNIX)
    add_definitions(-DCURL_STATICLIB -DCURL_USE_OPENSSL)
    set(CURL_USE_OPENSSL true CACHE INTERNAL "")
    set(HTTP_ONLY true CACHE INTERNAL "")
    set(CURL_STATICLIB true CACHE INTERNAL "")
    set(BUILD_CURL_EXE OFF CACHE INTERNAL "")
    add_subdirectory(curl EXCLUDE_FROM_ALL)
    include_directories(curl/include)
    link_directories(${CMAKE_BINARY_DIR}/curl/lib)
endif ()


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