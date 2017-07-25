cmake_minimum_required(VERSION 2.8)
project(consensus-yaraft)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules")

set(THIRDPARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build/third_parties)
set(YARAFT_THIRDPARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/yaraft/build/third_parties)
set(SOFA_PBRPC_THIRDPARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sofa-pbrpc/output)

# Look in thirdparty prefix paths before anywhere else for system dependencies.
set(CMAKE_PREFIX_PATH ${THIRDPARTY_DIR} ${YARAFT_THIRDPARTY_DIR} ${SOFA_PBRPC_THIRDPARTY_DIR} ${CMAKE_PREFIX_PATH})

find_package(Protobuf REQUIRED)

find_package(GLog REQUIRED)

find_package(Gflags REQUIRED)

find_package(Sanitizers REQUIRED)

find_library(GTEST_LIB gtest PATH ${DEPS_LIB_DIR})
find_library(GTEST_MAIN_LIB gtest_main PATH ${DEPS_LIB_DIR})
message("-- Found ${GTEST_LIB}")
message("-- Found ${GTEST_MAIN_LIB}")

find_package(Boost COMPONENTS system filesystem thread REQUIRED)

find_library(SILLY_LIBRARY silly_lite PATH ${DEPS_LIB_DIR})
message("-- Found ${SILLY_LIBRARY}")

find_library(FMT_LIBRARY fmt PATH ${DEPS_LIB_DIR})
message("-- Found ${FMT_LIBRARY}")

find_library(YARAFT_LIBRARY yaraft PATH ${DEPS_LIB_DIR})
message("-- Found ${YARAFT_LIBRARY}")

# required by sofa-pbrpc
find_library(SOFA_PBRPC_LIBRARY sofa-pbrpc PATH ${DEPS_LIB_DIR})
message("-- Found ${SOFA_PBRPC_LIBRARY}")
find_library(SNAPPY_LIBRARY snappy PATH ${DEPS_LIB_DIR})
message("-- Found ${SNAPPY_LIBRARY}")
find_package(ZLIB REQUIRED)

include_directories(${THIRDPARTY_DIR}/include)
include_directories(${YARAFT_THIRDPARTY_DIR}/include)
include_directories(${SOFA_PBRPC_THIRDPARTY_DIR}/include)
include_directories(src)

add_subdirectory(src)
