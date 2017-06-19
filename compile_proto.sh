#!/usr/bin/env bash

SCRIPT_DIR=$(cd `dirname $0` | pwd)
PROTOC=$SCRIPT_DIR/yaraft/build/third_parties/bin/protoc
$PROTOC -I $SCRIPT_DIR/yaraft/build/third_parties/include \
        -I $SCRIPT_DIR/src/rpc/pb \
        -I $SCRIPT_DIR/build/third_parties/include \
        --cpp_out=$SCRIPT_DIR/src/rpc/pb \
        $SCRIPT_DIR/src/rpc/pb/raft_server.proto