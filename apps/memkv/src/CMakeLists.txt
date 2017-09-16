set(MEMKV_LINK_LIBS
        ${FMT_LIBRARY}
        ${GLOG_STATIC_LIB}
        ${GFLAGS_STATIC_LIB}
        pthread
        )

set(MEMKV_SOURCES
        memkv.cc
        slice.h
        status.cc)

function(ADD_TEST TEST_NAME)
    add_executable(${TEST_NAME} ${TEST_NAME}.cc ${MEMKV_SOURCES})
    target_link_libraries(${TEST_NAME} ${MEMKV_LINK_LIBS} ${GTEST_LIB} ${GTEST_MAIN_LIB})
endfunction()

ADD_TEST(memkv_test)