cmake_minimum_required(VERSION 3.1)
project(Insight)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -std=c11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++14")

set(SOURCE_FILES
    src/insight.cc
    src/types.cc
    src/internal.hh
    src/cbridge/bridge.cc
    src/mangle.hh
    src/mangle.cc
)
set(INTERFACE_FILES
    include/insight/types.h
    include/insight/insight.h
    include/insight/types
    include/insight/insight
    include/insight/range
)

add_subdirectory(samples)

include_directories(include src)
add_library(insight ${SOURCE_FILES} ${INTERFACE_FILES})

link_directories(/usr/lib)
target_link_libraries(insight elf dwarf dwarf++)

install(FILES ${INTERFACE_FILES} DESTINATION include)
install(TARGETS insight
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
