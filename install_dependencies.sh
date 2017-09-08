#!/usr/bin/env bash

set -e

source deps_definitions.sh

echo "Installing necessary dependencies for building..."

# build yaraft
cd $PROJECT_DIR/yaraft
bash install_deps_if_necessary.sh
cd build && cmake .. -DCMAKE_INSTALL_PREFIX=$TP_BUILD_DIR -DBUILD_TEST=OFF && make -j8 && make install

# install sofa-pbrpc
cd $PROJECT_DIR

if [ ! -d $PROJECT_DIR/sofa-pbrpc ]; then
    fetch_and_expand $SOFA_PBRPC_NAME.zip
    mv $SOFA_PBRPC_NAME sofa-pbrpc
fi

if [ ! -d $PROJECT_DIR/sofa-pbrpc/output ]; then
    build_sofa_pbrpc
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

# install gflags
install_if_necessary $GFLAG_NAME build_gflag

# install google benchmark
install_if_necessary $GOOGLE_BENCH_NAME build_google_bench