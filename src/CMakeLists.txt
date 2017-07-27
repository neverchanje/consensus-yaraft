set(CONSENSUS_LINK_LIBS
        ${FMT_LIBRARY}
        ${SILLY_LIBRARY}
        ${GLOG_STATIC_LIB}
        ${GFLAGS_STATIC_LIB}
        ${Boost_LIBRARIES}
        ${YARAFT_LIBRARY}
        ${PROTOBUF_STATIC_LIBRARY}
        ${SOFA_PBRPC_LIBRARY}
        ${SNAPPY_LIBRARY}
        ${ZLIB_LIBRARIES}
        pthread)

##------------------- Base -------------------##

set(BASE_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src/base)

set(BASE_SOURCES
        ${BASE_SOURCE_DIR}/env_posix.cc
        ${BASE_SOURCE_DIR}/errno.cc
        ${BASE_SOURCE_DIR}/random.cc
        ${BASE_SOURCE_DIR}/status.cc
        ${BASE_SOURCE_DIR}/testing.cc
        ${BASE_SOURCE_DIR}/env_util.cc
        ${BASE_SOURCE_DIR}/coding.cc
        ${BASE_SOURCE_DIR}/endianness.cc
        ${BASE_SOURCE_DIR}/background_worker.cc
        ${BASE_SOURCE_DIR}/task_queue.cc)

add_library(consensus_base ${BASE_SOURCES})
target_link_libraries(consensus_base ${CONSENSUS_LINK_LIBS})
set(CONSENSUS_LINK_LIBS ${CONSENSUS_LINK_LIBS} consensus_base)

function(ADD_BASE_TEST TEST_NAME)
    add_executable(${TEST_NAME} ${BASE_SOURCE_DIR}/${TEST_NAME}.cc)
    target_link_libraries(${TEST_NAME} ${CONSENSUS_LINK_LIBS} ${GTEST_LIB} ${GTEST_MAIN_LIB})
endfunction()

ADD_BASE_TEST(env_test)

ADD_BASE_TEST(random_test)

ADD_BASE_TEST(coding_test)

ADD_BASE_TEST(background_worker_test)

##------------------- WAL -------------------##

set(WAL_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src/wal)

set(WAL_SOURCES
        ${WAL_SOURCE_DIR}/segment_meta.cc
        ${WAL_SOURCE_DIR}/options.cc
        ${WAL_SOURCE_DIR}/wal.cc
        ${WAL_SOURCE_DIR}/log_writer.cc
        ${WAL_SOURCE_DIR}/log_manager.cc)

add_library(consensus_wal ${WAL_SOURCES})
target_link_libraries(consensus_wal ${CONSENSUS_LINK_LIBS})
set(CONSENSUS_LINK_LIBS ${CONSENSUS_LINK_LIBS} consensus_wal)

function(ADD_WAL_TEST TEST_NAME)
    add_executable(${TEST_NAME} ${WAL_SOURCE_DIR}/${TEST_NAME}.cc)
    target_link_libraries(${TEST_NAME} ${CONSENSUS_LINK_LIBS} ${GTEST_LIB} ${GTEST_MAIN_LIB})
endfunction()

ADD_WAL_TEST(segment_meta_test)

ADD_WAL_TEST(log_writer_test)

ADD_WAL_TEST(log_manager_test)

##------------------- RPC -------------------##

set(RPC_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src/rpc)

set(RPC_SOURCES
        ${RPC_SOURCE_DIR}/peer.cc
        ${RPC_SOURCE_DIR}/cluster.cc
)

add_library(consensus_rpc ${RPC_SOURCES})
target_link_libraries(consensus_rpc ${CONSENSUS_LINK_LIBS})
set(CONSENSUS_LINK_LIBS ${CONSENSUS_LINK_LIBS} consensus_rpc)

##------------------- consensus-all -------------------##

set(CONSENSUS_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)

set(ALL_SOURCES
        ${CONSENSUS_SOURCE_DIR}/replicated_log.cc
        ${CONSENSUS_SOURCE_DIR}/replicated_log_impl.h
        ${CONSENSUS_SOURCE_DIR}/ready_flusher.cc
        ${CONSENSUS_SOURCE_DIR}/raft_timer.cc
        ${CONSENSUS_SOURCE_DIR}/raft_task_executor.cc
        ${CONSENSUS_SOURCE_DIR}/wal_commit_observer.cc
        ${CONSENSUS_SOURCE_DIR}/raft_service.cc
        ${CONSENSUS_SOURCE_DIR}/pb/raft_server.pb.cc)
add_library(consensus_yaraft ${ALL_SOURCES})
target_link_libraries(consensus_yaraft ${CONSENSUS_LINK_LIBS})
set(CONSENSUS_LINK_LIBS ${CONSENSUS_LINK_LIBS} consensus_yaraft)

function(ADD_CONSENSUS_TEST TEST_NAME)
    add_executable(${TEST_NAME} ${CONSENSUS_SOURCE_DIR}/${TEST_NAME}.cc)
    target_link_libraries(${TEST_NAME} ${CONSENSUS_LINK_LIBS} ${GTEST_LIB} ${GTEST_MAIN_LIB})
endfunction()

ADD_CONSENSUS_TEST(raft_task_executor_test)
ADD_CONSENSUS_TEST(raft_timer_test)
ADD_CONSENSUS_TEST(ready_flusher_test)
ADD_CONSENSUS_TEST(raft_service_test)
ADD_CONSENSUS_TEST(replicated_log_test)

set(CONSENSUS_YARAFT_INSTALL_HEADERS
        replicated_log.h
        raft_service.h
        wal/wal.h
        rpc/cluster.h
        pb/raft_server.pb.h
        base/logging.h)

install(TARGETS consensus_yaraft DESTINATION lib)
install(FILES ${CONSENSUS_YARAFT_INSTALL_HEADERS} DESTINATION include/yaraft)

##------------------- raft_server -------------------##

add_executable(raft_server raft_server_main.cc)
target_link_libraries(raft_server ${CONSENSUS_LINK_LIBS})
install(TARGETS raft_server DESTINATION bin)