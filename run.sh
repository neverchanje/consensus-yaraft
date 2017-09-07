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
    echo
    echo "   start_onebox              start yaraft onebox"
    echo "   stop_onebox               stop yaraft onebox"
    echo "   list_onebox               list yaraft onebox"
    echo
    echo "   test                      run unit test"
    echo
    echo "Command 'run.sh <command> -h' will print help for subcommands."
}

function run_build() {
    pushd ${PROJECT_DIR}
    mkdir -p ${PROJECT_DIR}/cmake-build-debug
    mkdir -p ${PROJECT_DIR}/output
    cd ${PROJECT_DIR}/cmake-build-debug
    cmake .. -DCMAKE_INSTALL_PREFIX=${PROJECT_DIR}/output -DCMAKE_BUILD_TYPE=Release
    make -j8 && make install
    popd
}

#####################
## start_onebox
#####################

function usage_start_onebox()
{
    echo "Options for subcommand 'start_onebox':"
    echo "   -h|--help         print the help info"
    echo "   -m|--server_count <num>"
    echo "                     server count, default is 3"
}

function run_start_onebox() {
    cd ${PROJECT_DIR}
    SERVER_COUNT=3
    while [[ $# > 0 ]]; do
        key="$1"
        case $key in
            -h|--help)
                usage_start_onebox
                exit 0
                ;;
            -s|--server_count)
                SERVER_COUNT="$2"
                shift
                ;;
            *)
                echo "ERROR: unknown option \"$key\""
                echo
                usage_start_onebox
                exit -1
                ;;
        esac
        shift
    done

    port=12320
    for i in $(seq ${SERVER_COUNT})
    do
        # restart from empty wal dir
        if [ -d ./output/infra${i}.consensus ]; then
            rm -rf ./output/infra${i}.consensus
        fi
        ./output/bin/raft_server \
            --id=${i} \
            --server_num=${SERVER_COUNT} \
            --wal_dir=${PROJECT_DIR}/output/infra${i}.consensus  &>./output/bin/server${i}.log &
    done
}

function run_stop_onebox() {
    ps -ef | grep output/bin/raft_server | grep 'server_num' | awk '{print $2}' | xargs kill &>/dev/null
}

function run_list_onebox() {
    ps -ef | grep output/bin/raft_server | grep 'server_num' | sort -k11
}

#####################
## stop_onebox_instance
#####################

function usage_stop_onebox_instance()
{
    echo "Options for subcommand 'stop_onebox_instance':"
    echo "   -h|--help         print the help info"
    echo "   -s|--server_id    <num>"
    echo "                     raft server id"
}

function run_stop_onebox_instance() {
    SERVER_ID=0
    while [[ $# > 0 ]]; do
        key="$1"
        case $key in
            -h|--help)
                usage_stop_onebox_instance
                exit 0
                ;;
            -s|--server_id)
                SERVER_ID="$2"
                shift
                ;;
            *)
                echo "ERROR: unknown option \"$key\""
                echo
                usage_stop_onebox_instance
                exit -1
                ;;
        esac
        shift
    done
    if [ ${SERVER_ID} != "0" ]; then
        ps -ef | grep output/bin/raft_server | grep id=${SERVER_ID} | awk '{print $2}' | xargs kill &>/dev/null
    fi
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

    unit_test segment_meta_test
    unit_test log_writer_test
    unit_test log_manager_test

    unit_test raft_service_test
    unit_test ready_flusher_test
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
    start_onebox)
        shift
        run_start_onebox $*
        ;;
    stop_onebox)
        shift
        run_stop_onebox $*
        ;;
    list_onebox)
        shift
        run_list_onebox $*
        ;;
    stop_onebox_instance)
        shift
        run_stop_onebox_instance $*
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
