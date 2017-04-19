#!/usr/bin/env bash

set -e

source deps_definitions.sh

echo "Installing necessary dependencies for building..."

# build yaraft
cd yaraft
bash install_deps_if_necessary.sh
cd build && cmake .. -DCMAKE_INSTALL_PREFIX=$TP_BUILD_DIR && make -j8 && make install

mkdir -p $BUILD_DIR
cd $PROJECT_DIR && cp -r yaraft/build/third_parties $BUILD_DIR

# install other dependencies
mkdir -p $TP_DIR
mkdir -p $TP_STAMP_DIR

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
install_if_necessary $GFLAG_NAME build_gflag
