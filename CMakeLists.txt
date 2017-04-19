cmake_minimum_required(VERSION 2.8)
project(yaraft_wal)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules")

set(THIRDPARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build/third_parties)

# Look in thirdparty prefix paths before anywhere else for system dependencies.
set(CMAKE_PREFIX_PATH ${THIRDPARTY_DIR} ${CMAKE_PREFIX_PATH})

## Protobuf
find_package(Protobuf REQUIRED)
include_directories(SYSTEM ${PROTOBUF_INCLUDE_DIR})

## GLog
find_package(GLog REQUIRED)
include_directories(SYSTEM ${GLOG_INCLUDE_DIR})

## GFlags
find_package(Gflags REQUIRED)
include_directories(SYSTEM ${GFLAGS_INCLUDE_DIR})

## GTest
find_library(GTEST_LIB gtest PATH ${DEPS_LIB_DIR})
find_library(GTEST_MAIN_LIB gtest_main PATH ${DEPS_LIB_DIR})
message("-- Found ${GTEST_LIB}")
message("-- Found ${GTEST_MAIN_LIB}")

## Boost Filesystem
find_package(Boost COMPONENTS system filesystem REQUIRED)

find_library(SILLY_LIBRARY silly_lite PATH ${DEPS_LIB_DIR})
message("-- Found ${SILLY_LIBRARY}")

find_library(FMT_LIBRARY fmt PATH ${DEPS_LIB_DIR})
message("-- Found ${FMT_LIBRARY}")

include_directories(build/third_parties/include)
include_directories(src)

add_subdirectory(src)

file(GLOB YARAFT_SOURCE "src/*/*.h" "src/base/status.cc")
add_library(fuck_clion ${YARAFT_SOURCE})