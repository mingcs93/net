# set minimum cmake version
cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

# project name and language
project(mingcs LANGUAGES CXX)
set(target mingcs)

# require C++17
set(CMAKE_CXX_STANDARD 17)

if(WIN32)
else()
SET(CMAKE_BUILD_TYPE "Debug")

SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")

SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
endif()


set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(BASE_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/base")
set(NET_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/net")

option(TEST "option for test" OFF)

add_subdirectory(base)
add_subdirectory(net)
if(TEST)
add_subdirectory(test)
endif()