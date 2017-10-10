#!/usr/bin/env bash

PROJECT_DIR=`pwd`
BUILD_DIR=$PROJECT_DIR/build
TP_DIR=$PROJECT_DIR/third_parties
TP_BUILD_DIR="$BUILD_DIR/third_parties"
TP_STAMP_DIR=$TP_BUILD_DIR/stamp

SNAPPY_VERSION=1.1.3
SNAPPY_NAME=snappy-$SNAPPY_VERSION
SNAPPY_SOURCE=$TP_DIR/$SNAPPY_NAME

BRPC_VERSION=`git rev-parse @:brpc | cut -c 1-7`
BRPC_NAME=brpc-$BRPC_VERSION
BRPC_SOURCE=$PROJECT_DIR/brpc

GOOGLE_BENCH_VERSION=1.2.0
GOOGLE_BENCH_NAME=benchmark-$GOOGLE_BENCH_VERSION
GOOGLE_BENCH_SOURCE=$TP_DIR/$GOOGLE_BENCH_NAME

GLOG_VERSION=0.3.5
GLOG_NAME=glog-$GLOG_VERSION
GLOG_SOURCE=$TP_DIR/$GLOG_NAME

GFLAG_VERSION=2.2.0
GFLAG_NAME=gflags-$GFLAG_VERSION
GFLAG_SOURCE=$TP_DIR/$GFLAG_NAME

QINIU_CDN_URL_PREFIX=http://onnzg1pyx.bkt.clouddn.com

make_stamp() {
  touch $TP_STAMP_DIR/$1
}

fetch_and_expand() {
  local FILENAME=$1
  if [ -z "$FILENAME" ]; then
    echo "Error: Must specify file to fetch"
    exit 1
  fi

  TAR_CMD=tar
  if [[ "$OSTYPE" == "darwin"* ]] && which gtar &>/dev/null; then
    TAR_CMD=gtar
  fi

  FULL_URL="${QINIU_CDN_URL_PREFIX}/${FILENAME}"
  SUCCESS=0
  # Loop in case we encounter a corrupted archive and we need to re-download it.
  for attempt in 1 2; do
    if [ -r "$FILENAME" ]; then
      echo "Archive $FILENAME already exists. Not re-downloading archive."
    else
      echo "Fetching $FILENAME from $FULL_URL"
      wget "$FULL_URL"
    fi

    echo "Unpacking $FILENAME"
    if [[ "$FILENAME" =~ \.zip$ ]]; then
      if ! unzip -q "$FILENAME"; then
        echo "Error unzipping $FILENAME, removing file"
        rm "$FILENAME"
        continue
      fi
    elif [[ "$FILENAME" =~ \.(tar\.gz|tgz)$ ]]; then
      if ! $TAR_CMD xf "$FILENAME"; then
        echo "Error untarring $FILENAME, removing file"
        rm "$FILENAME"
        continue
      fi
    else
      echo "Error: unknown file format: $FILENAME"
      exit 1
    fi

    SUCCESS=1
    break
  done

  if [ $SUCCESS -ne 1 ]; then
    echo "Error: failed to fetch and unpack $FILENAME"
    exit 1
  fi

  # Allow for not removing previously-downloaded artifacts.
  # Useful on a low-bandwidth connection.
  if [ -z "$NO_REMOVE_THIRDPARTY_ARCHIVES" ]; then
    echo "Removing $FILENAME"
    rm $FILENAME
  fi
  echo
}

build_brpc() {
  pushd $BRPC_SOURCE
  bash $PROJECT_DIR/build_brpc.sh
  popd
}

build_google_bench() {
  mkdir -p $GOOGLE_BENCH_SOURCE/build
  pushd $GOOGLE_BENCH_SOURCE/build
  rm -rf CMakeCache.txt CMakeFiles/
  cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_POSITION_INDEPENDENT_CODE=On \
    -DCMAKE_INSTALL_PREFIX=$TP_BUILD_DIR \
    -DBUILD_SHARED_LIBS=On \
    -DBUILD_STATIC_LIBS=On \
    -DREGISTER_INSTALL_PREFIX=Off \
    $GOOGLE_BENCH_SOURCE
  make -j8 install
  popd
}

build_gflag() {
  GFLAGS_BDIR=$GFLAG_SOURCE/build
  mkdir -p $GFLAGS_BDIR
  pushd $GFLAGS_BDIR
  rm -rf CMakeCache.txt CMakeFiles/
  cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_POSITION_INDEPENDENT_CODE=On \
    -DCMAKE_INSTALL_PREFIX=$TP_BUILD_DIR \
    -DBUILD_SHARED_LIBS=On \
    -DBUILD_STATIC_LIBS=On \
    -DREGISTER_INSTALL_PREFIX=Off \
    $GFLAG_SOURCE
  make install
  popd
}

build_glog() {
  GLOG_BDIR=$GLOG_SOURCE/build
  mkdir -p $GLOG_BDIR
  pushd ${GLOG_BDIR}
  cmake \
    -DCMAKE_INSTALL_PREFIX=$TP_BUILD_DIR \
    -DBUILD_SHARED_LIBS=Off \
    -DWITH_GFLAGS=OFF \
    -DBUILD_TESTING=OFF \
    $GLOG_SOURCE
  make -j4 && make install
  popd
}