#!/usr/bin/env bash

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
    cmake .. -DCMAKE_INSTALL_PREFIX=${PROJECT_DIR}/output
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
        port=$((${port}+1))
        initial_cluster="${initial_cluster}infra${i}=127.0.0.1:${port};"
    done

    for i in $(seq ${SERVER_COUNT})
    do
        ./output/bin/raft_server \
            --initial_cluster=${initial_cluster} \
            --name=infra${i} \
            --wal_dir=${PROJECT_DIR}/output/infra${i}.consensus  &>./output/bin/server${i}.log &
    done
}

function run_stop_onebox() {
    ps -ef | grep output/bin/raft_server | grep 'initial_cluster' | awk '{print $2}' | xargs kill &>/dev/null
}

function run_list_onebox() {
    ps -ef | grep output/bin/raft_server | grep 'initial_cluster' | sort -k11
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
        ps -ef | grep output/bin/raft_server | grep name=infra${SERVER_ID} | awk '{print $2}' | xargs kill &>/dev/null
    fi
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