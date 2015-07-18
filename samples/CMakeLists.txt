cmake_minimum_required(VERSION 3.1)
project(Insight_samples)

include_directories(../include)

add_executable(struct struct.c)
target_link_libraries(struct insight)

add_executable(classes classes.cc)
target_link_libraries(classes insight)
