# set minimum cmake version
cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

# project name and language
project(net LANGUAGES CXX)
set(target net)

include_directories(${BASE_INCLUDE_PATH})

file(GLOB INCLUDE_LIST "*.h")
aux_source_directory(. SRC_LIST)

add_definitions(-DNET_EXPORTS)

add_library(${target} SHARED ${INCLUDE_LIST} ${SRC_LIST}) 

add_dependencies(${target} baseCommon)

target_link_libraries(${target} baseCommon)