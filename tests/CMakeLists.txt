cmake_minimum_required(VERSION 3.1)
project(Insight_tests)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-multichar")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-multichar")
include_directories(../include)

add_executable(test_insight test.cc virtual.cc typeof.cc class.cc union.cc annotation.cc enum.cc)
target_link_libraries(test_insight insight gtest)
