
set(CONSENSUS_TEST_LINK_LIBS
        ${GTEST_LIB}
        ${GTEST_MAIN_LIB}
        ${FMT_LIBRARY}
        ${SILLY_LIBRARY}
        ${GLOG_STATIC_LIB}
        ${GFLAGS_STATIC_LIB}
        ${Boost_LIBRARIES}
        ${YARAFT_LIBRARY}
        ${PROTOBUF_STATIC_LIBRARY}
        pthread)

set(BASE_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src/base)
set(WAL_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src/wal)

set(BASE_SOURCES
        ${BASE_SOURCE_DIR}/env_posix.cc
        ${BASE_SOURCE_DIR}/errno.cc
        ${BASE_SOURCE_DIR}/random.cc
        ${BASE_SOURCE_DIR}/status.cc
        ${BASE_SOURCE_DIR}/testing.cc
        ${BASE_SOURCE_DIR}/env_util.cc
        ${BASE_SOURCE_DIR}/coding.cc
        ${BASE_SOURCE_DIR}/endianness.cc)

set(WAL_SOURCES
        ${WAL_SOURCE_DIR}/segment_meta.cc
        ${WAL_SOURCE_DIR}/options.cc
        ${WAL_SOURCE_DIR}/log_writer.cc
        ${WAL_SOURCE_DIR}/log_manager.cc)

function(ADD_BASE_TEST TEST_NAME)
    add_executable(${TEST_NAME}
            ${BASE_SOURCE_DIR}/${TEST_NAME}.cc
            ${BASE_SOURCES})
    target_link_libraries(${TEST_NAME} ${CONSENSUS_TEST_LINK_LIBS})
endfunction()

function(ADD_WAL_TEST TEST_NAME)
    add_executable(${TEST_NAME}
            ${WAL_SOURCE_DIR}/${TEST_NAME}.cc
            ${BASE_SOURCES}
            ${WAL_SOURCES})
    target_link_libraries(${TEST_NAME} ${CONSENSUS_TEST_LINK_LIBS})
endfunction()

ADD_BASE_TEST(env_test)

ADD_BASE_TEST(random_test)

ADD_BASE_TEST(coding_test)

ADD_WAL_TEST(segment_meta_test)

ADD_WAL_TEST(log_writer_test)

ADD_WAL_TEST(log_manager_test)