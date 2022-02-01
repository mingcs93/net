# set minimum cmake version
cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

# project name and language
project(timestampTest LANGUAGES CXX)
set(target timestampTest)


include_directories(${BASE_INCLUDE_PATH})

aux_source_directory(. SRC_LIST)

add_executable(${target} ${SRC_LIST})

set_target_properties(${target} PROPERTIES FOLDER "test")

add_dependencies(${target} baseCommon)

target_link_libraries(${target} baseCommon)