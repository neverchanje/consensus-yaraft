cmake_minimum_required(VERSION 2.8)

project(memkv)

get_filename_component(CONSENSUS_YARAFT_DIR ${PROJECT_SOURCE_DIR} PATH)
get_filename_component(CONSENSUS_YARAFT_DIR ${CONSENSUS_YARAFT_DIR} PATH)

set(YARAFT_THIRDPARTY_DIR ${CONSENSUS_YARAFT_DIR}/yaraft/build/third_parties)
set(CONSENSUS_YARAFT_THIRDPARTY_DIR ${CONSENSUS_YARAFT_DIR}/build/third_parties)

set(CMAKE_PREFIX_PATH ${THIRDPARTY_DIR} ${YARAFT_THIRDPARTY_DIR} ${CMAKE_PREFIX_PATH})

set(MEMKV_SOURCES
        src/memkv.h
        src/slice.h
        src/status.cc)

include_directories(${YARAFT_THIRDPARTY_DIR}/include)
include_directories(${CONSENSUS_YARAFT_THIRDPARTY_DIR}/include)

add_executable(memkv ${MEMKV_SOURCES})