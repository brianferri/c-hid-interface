cmake_minimum_required(VERSION 3.4.3...3.25 FATAL_ERROR)

get_filename_component(ProjectId ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${ProjectId} LANGUAGES C)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG_LOGGING)
    message(STATUS "Debug logging is enabled.")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)

foreach(config IN ITEMS Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER ${config} CONFIG)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG} ${CMAKE_BINARY_DIR}/output/${config})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG} ${CMAKE_BINARY_DIR}/output/${config})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG} ${CMAKE_BINARY_DIR}/output/${config})
endforeach()

set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS $<$<CONFIG:Debug>:DEBUG_LOGGING>)

message(STATUS " ===== Setting up project =====")
add_executable(${CMAKE_PROJECT_NAME} ./src/_init/main.c)

message(STATUS " ===== Setting compiler options =====")
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS "Windows detected.")
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:/Wall>)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(STATUS "Linux detected.")
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:-Wall>)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    message(STATUS "MacOS detected.")
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:-Wall>)
endif()

message(STATUS " ===== Setting include directories =====")
set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR}/output)
# HIDAPI
set(HIDAPI_WITH_HIDRAW OFF)

message(STATUS " ===== Adding include directories =====")
add_subdirectory(./include/lib/hidapi)

find_package(json-c CONFIG QUIET)

if(NOT json-c_FOUND)
    message(STATUS "json-c not found, adding as a subdirectory...")
    add_subdirectory(./include/lib/json-c)
    set(JSONC_TARGET json-c)
else()
    set(JSONC_TARGET json-c::json-c)
endif()

message(STATUS " ===== Linking libraries =====")
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE hidapi::hidapi ${JSONC_TARGET})
