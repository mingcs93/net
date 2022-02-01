# set minimum cmake version
cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

# project name and language
project(tcpClientTest LANGUAGES CXX)
set(target tcpClientTest)


include_directories(${BASE_INCLUDE_PATH})
include_directories(${NET_INCLUDE_PATH})

aux_source_directory(. SRC_LIST)

add_executable(${target} ${SRC_LIST})

set_target_properties(${target} PROPERTIES FOLDER "test")

add_dependencies(${target} baseCommon)
add_dependencies(${target} net)

target_link_libraries(${target} baseCommon)
target_link_libraries(${target} net)