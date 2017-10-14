#!/usr/bin/env bash

set -e

PROJECT_DIR=`pwd`

function usage()
{
    echo "usage: run.sh <command> [<args>]"
    echo
    echo "Command list:"
    echo "   help                      print the help info"
    echo "   build                     build the system"
    echo "   test                      run unit test"
    echo
    echo "Command 'run.sh <command> -h' will print help for subcommands."
}

#####################
## build
#####################

function run_build() {
    pushd ${PROJECT_DIR}
    mkdir -p ${PROJECT_DIR}/cmake-build-debug
    mkdir -p ${PROJECT_DIR}/output
    cd ${PROJECT_DIR}/cmake-build-debug
    cmake .. -DCMAKE_INSTALL_PREFIX=${PROJECT_DIR}/output -DCMAKE_BUILD_TYPE=Debug
    make -j8 && make install
    popd
}

#####################
## unit test
#####################

TEST_DIR=cmake-build-debug/src

function unit_test()
{
    echo "===========" $1 "==========="
    ./$TEST_DIR/$1
    if [ $? -ne 0 ]; then
        echo "TEST FAILED!!!"
        exit 1
    fi
}


function run_test() {
    unit_test env_test
    unit_test coding_test
    unit_test background_worker_test
    unit_test random_test

    unit_test log_writer_test
    unit_test log_manager_test

    unit_test raft_service_test
    unit_test raft_timer_test
    unit_test raft_task_executor_test
    unit_test replicated_log_test
}

####################################################################

if [ $# -eq 0 ]; then
    usage
    exit 0
fi

cmd=$1
case $cmd in
    help)
        usage
        ;;
    build)
        shift
        run_build $*
        ;;
    test)
        shift
        run_test $*
        ;;
    *)
        echo "ERROR: unknown command $cmd"
        echo
        usage
        exit -1
esac
