
set(CONSENSUS_TEST_LINK_LIBS
        ${GTEST_LIB}
        ${GTEST_MAIN_LIB}
        ${FMT_LIBRARY}
        ${SILLY_LIBRARY}
        ${GLOG_STATIC_LIB}
        ${GFLAGS_STATIC_LIB}
        ${Boost_LIBRARIES}
        pthread)

set(BASE_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src/base)

function(ADD_BASE_TEST TEST_NAME)
    add_executable(${TEST_NAME}
            ${BASE_SOURCE_DIR}/${TEST_NAME}.cc
            ${BASE_SOURCE_DIR}/env_posix.cc
            ${BASE_SOURCE_DIR}/errno.cc
            ${BASE_SOURCE_DIR}/random.cc
            ${BASE_SOURCE_DIR}/status.cc
            ${BASE_SOURCE_DIR}/testing.cc)
    target_link_libraries(${TEST_NAME} ${CONSENSUS_TEST_LINK_LIBS})
endfunction()

ADD_BASE_TEST(env_test)

ADD_BASE_TEST(random_test)