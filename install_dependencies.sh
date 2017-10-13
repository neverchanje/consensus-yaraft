#!/usr/bin/env bash

set -e

source deps_definitions.sh

echo "Installing necessary dependencies for building..."

# build yaraft
build_yaraft

# build brpc
if [ ! -f $TP_STAMP_DIR/$BRPC_NAME ]; then
    build_brpc
    make_stamp $BRPC_NAME
fi

# install other dependencies
cd $PROJECT_DIR
mkdir -p $TP_DIR
mkdir -p $TP_STAMP_DIR
mkdir -p $BUILD_DIR

install_if_necessary(){
    local depName=$1
    if [ ! -d $TP_DIR/$depName ]; then
        fetch_and_expand $depName.zip
    fi
    if [ ! -f $TP_STAMP_DIR/$depName ]; then
        $2
        make_stamp $depName
    fi
}

cd $TP_DIR

# install google benchmark
install_if_necessary $GOOGLE_BENCH_NAME build_google_bench

install_if_necessary $GFLAG_NAME build_gflag
install_if_necessary $GLOG_NAME build_glog