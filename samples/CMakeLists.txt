cmake_minimum_required(VERSION 3.1)
project(Insight_samples)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -std=c11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++14")
include_directories(../include)

add_executable(struct struct.c)
target_link_libraries(struct insight)

add_executable(classes classes.cc)
target_link_libraries(classes insight)
