#!/usr/bin/env bash

SCRIPT_DIR=$(cd `dirname $0` | pwd)
PROTOC=$SCRIPT_DIR/yaraft/build/third_parties/bin/protoc
$PROTOC -I $SCRIPT_DIR/yaraft/build/third_parties/include \
        -I $SCRIPT_DIR/src/pb \
        -I $SCRIPT_DIR/build/third_parties/include \
        --cpp_out=$SCRIPT_DIR/src/pb \
        $SCRIPT_DIR/src/pb/raft_server.proto