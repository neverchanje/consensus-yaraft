#!/usr/bin/env bash

SCRIPT_DIR=$(realpath `dirname $0`)
CONSENSUS_YARAFT_DIR=$(dirname `dirname $SCRIPT_DIR`)
PROTOC=$CONSENSUS_YARAFT_DIR/yaraft/build/third_parties/bin/protoc

$PROTOC -I $SCRIPT_DIR/src/pb \
        --cpp_out=$SCRIPT_DIR/src/pb \
        $SCRIPT_DIR/src/pb/memkv.proto