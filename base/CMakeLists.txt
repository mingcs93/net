# set minimum cmake version
cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

# project name and language
if(WIN32)
set(target baseCommon)
else()
set(target baseCommon)
endif()

project(baseCommon LANGUAGES CXX)

file(GLOB INCLUDE_LIST "*.h")
aux_source_directory(. SRC_LIST)

add_definitions(-DBASE_EXPORTS)

add_library(${target} SHARED ${INCLUDE_LIST} ${SRC_LIST})

if(WIN32)
else()
set_target_properties(${target} PROPERTIES
COMPILE_FLAGS "-pthread"
LINK_FLAGS "-pthread")
endif()