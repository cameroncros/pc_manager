cmake_minimum_required(VERSION 3.20)

project(pc_manager VERSION 1.15.0)

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

configure_file(src/inc/version.h.in ${CMAKE_BINARY_DIR}/generated/inc/version.h @ONLY)
include_directories(${CMAKE_BINARY_DIR}/generated/inc/)

set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
set(LINK_SEARCH_START_STATIC TRUE)
set(LINK_SEARCH_END_STATIC TRUE)

enable_testing()

add_subdirectory(external)
add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(install)
add_subdirectory(integration)
