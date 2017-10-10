cmake_minimum_required(VERSION 2.8)

project(memkv)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

get_filename_component(CONSENSUS_YARAFT_DIR ${PROJECT_SOURCE_DIR} PATH)
get_filename_component(CONSENSUS_YARAFT_DIR ${CONSENSUS_YARAFT_DIR} PATH)

set(YARAFT_THIRDPARTY_DIR ${CONSENSUS_YARAFT_DIR}/yaraft/build/third_parties)
set(CONSENSUS_YARAFT_THIRDPARTY_DIR ${CONSENSUS_YARAFT_DIR}/build/third_parties)
set(CONSENSUS_YARAFT_OUTPUT ${CONSENSUS_YARAFT_DIR}/output)
set(BRPC_THIRDPARTY_DIR ${CONSENSUS_YARAFT_DIR}/brpc/output)

set(CMAKE_PREFIX_PATH
        ${CONSENSUS_YARAFT_OUTPUT}
        ${CONSENSUS_YARAFT_THIRDPARTY_DIR}
        ${YARAFT_THIRDPARTY_DIR}
        ${BRPC_THIRDPARTY_DIR}
        ${CMAKE_PREFIX_PATH})
set(CMAKE_MODULE_PATH "${CONSENSUS_YARAFT_DIR}/cmake_modules")

find_package(Boost COMPONENTS system filesystem thread REQUIRED)
find_package(Protobuf REQUIRED)
find_package(Gflags REQUIRED)

find_library(FMT_LIBRARY fmt)
message("-- Found ${FMT_LIBRARY}")

find_library(GTEST_LIB gtest)
find_library(GTEST_MAIN_LIB gtest_main)
message("-- Found ${GTEST_LIB}")
message("-- Found ${GTEST_MAIN_LIB}")

find_library(BRPC_LIBRARY libbrpc.a brpc)
message("-- Found ${BRPC_LIBRARY}")

find_package(ZLIB REQUIRED)
find_package(GLog REQUIRED)
find_package(OpenSSL REQUIRED)

find_library(LEVELDB_LIBRARY leveldb)
message("-- Found ${LEVELDB_LIBRARY}")

find_library(CONSENSUS_YARAFT_LIBRARY consensus_yaraft)
message("-- Found ${CONSENSUS_YARAFT_LIBRARY}")
find_library(YARAFT_LIBRARY yaraft)
message("-- Found ${YARAFT_LIBRARY}")

include_directories(${YARAFT_THIRDPARTY_DIR}/include)
include_directories(${CONSENSUS_YARAFT_THIRDPARTY_DIR}/include)
include_directories(${CONSENSUS_YARAFT_DIR}/output/include)
include_directories(${BRPC_THIRDPARTY_DIR}/include)

add_subdirectory(src)