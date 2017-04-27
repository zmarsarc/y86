cmake_minimum_required(VERSION 2.8)

project(yvm_test)

include_directories(../include)
link_directories(${PROJECT_SOURCE_DIR}/../lib)

find_file(TARGET_TEST_FILE yvm.cpp ../src)
aux_source_directory(. SRCS)

link_libraries(gtest)
add_compile_options(/MT /EHsc)
add_executable(yvm_test ${TARGET_TEST_FILE} ${SRCS})