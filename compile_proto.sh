#!/usr/bin/env bash

SCRIPT_DIR=$(cd `dirname $0` | pwd)
PROTOC=$SCRIPT_DIR/yaraft/build/third_parties/bin/protoc
CONSENSUS_PROTO_DIR=$SCRIPT_DIR/include/consensus/pb
$PROTOC -I $SCRIPT_DIR/yaraft/build/third_parties/include \
        -I $CONSENSUS_PROTO_DIR \
        -I $SCRIPT_DIR/build/third_parties/include \
        --cpp_out=$CONSENSUS_PROTO_DIR \
        $CONSENSUS_PROTO_DIR/raft_server.proto