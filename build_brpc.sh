#!/usr/bin/env bash

SCRIPT_DIR=$(realpath `dirname $0`)
BRPC_CONFIG_SCRIPT=$SCRIPT_DIR/brpc/config_brpc.sh

GFLAGS_DEP_DIR=$SCRIPT_DIR/build/third_parties
GFLAGS_HEADER_DIR=$GFLAGS_DEP_DIR/include
GFLAGS_LIBS_DIR=$GFLAGS_DEP_DIR/lib

PROTOBUF_DEP_DIR=$SCRIPT_DIR/yaraft/build/third_parties
PROTOBUF_HEADER_DIR=$PROTOBUF_DEP_DIR/include
PROTOBUF_LIBS_DIR=$PROTOBUF_DEP_DIR/lib

# leveldb-dev should have been installed.

bash $BRPC_CONFIG_SCRIPT --headers="$GFLAGS_HEADER_DIR $PROTOBUF_HEADER_DIR /usr/include" \
    --libs="$GFLAGS_LIBS_DIR $PROTOBUF_LIBS_DIR /usr/lib64" --cxx=g++ --cc=gcc
make -j4