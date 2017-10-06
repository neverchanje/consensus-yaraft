cmake_minimum_required(VERSION 2.8)
project(consensus-yaraft)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules")

set(THIRDPARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build/third_parties)
set(YARAFT_THIRDPARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/yaraft/build/third_parties)
set(BRPC_THIRDPARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/brpc/output)

# Look in thirdparty prefix paths before anywhere else for system dependencies.
set(CMAKE_PREFIX_PATH ${THIRDPARTY_DIR} ${YARAFT_THIRDPARTY_DIR} ${BRPC_THIRDPARTY_DIR} ${CMAKE_PREFIX_PATH})

find_package(Protobuf REQUIRED)

find_package(GLog REQUIRED)

find_package(Gflags REQUIRED)

find_package(Sanitizers REQUIRED)

find_library(GTEST_LIB gtest PATH ${DEPS_LIB_DIR})
find_library(GTEST_MAIN_LIB gtest_main PATH ${DEPS_LIB_DIR})
message("-- Found ${GTEST_LIB}")
message("-- Found ${GTEST_MAIN_LIB}")

find_package(Boost COMPONENTS system filesystem thread REQUIRED)

find_package(OpenSSL REQUIRED)

find_library(FMT_LIBRARY fmt PATH ${DEPS_LIB_DIR})
message("-- Found ${FMT_LIBRARY}")

find_library(YARAFT_LIBRARY yaraft PATH ${DEPS_LIB_DIR})
message("-- Found ${YARAFT_LIBRARY}")

find_library(BRPC_LIBRARY libbrpc.a brpc)
message("-- Found ${BRPC_LIBRARY}")
find_library(LEVELDB_LIBRARY leveldb)
message("-- Found ${LEVELDB_LIBRARY}")
find_package(ZLIB REQUIRED)

include_directories(${THIRDPARTY_DIR}/include)
include_directories(${YARAFT_THIRDPARTY_DIR}/include)
include_directories(${BRPC_THIRDPARTY_DIR}/include)
include_directories(src)
include_directories(include)
include_directories(include/consensus)

add_subdirectory(src)