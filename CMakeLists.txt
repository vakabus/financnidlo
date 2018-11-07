cmake_minimum_required(VERSION 3.12)
project(financnidlo6)

set(CMAKE_CXX_STANDARD 17)

include_directories(src)
include_directories(tests)

add_executable(financnidlo6
        src/balancer.h
        src/iterator.h
        src/main.cpp
        src/model.h
        src/parser.h
        src/types.h)

add_executable(financnidlo6-test
        tests/main.cpp
        tests/test_iterator.h
        tests/test_parser.h)
target_link_libraries(financnidlo6-test gtest gtest_main)