cmake_minimum_required(VERSION 3.1)
project(Insight)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/.cmake/Modules/")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -g -std=c11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -g -std=c++14")

set(SOURCE_FILES
    src/core/dwarf.cc
    src/data/types.cc
    src/data/internal.hh
    src/cbridge/bridge.cc
    src/util/mangle.hh
    src/util/mangle.cc
)
set(INTERFACE_FILES
    include/insight/types.h
    include/insight/insight.h
    include/insight/types
    include/insight/insight
    include/insight/range
    include/insight/compare
    include/insight/compare.hxx
    include/insight/stream.hxx
)

add_subdirectory(samples)
add_subdirectory(tests)

include_directories(include src)
add_library(insight SHARED ${SOURCE_FILES} ${INTERFACE_FILES})

link_directories(/usr/lib)
target_link_libraries(insight elf dwarf dwarf++)

install(FILES ${INTERFACE_FILES} DESTINATION include/insight)
install(TARGETS insight
  LIBRARY DESTINATION lib
)

add_custom_target(uninstall
    "${CMAKE_COMMAND}" -P "${CMAKE_MODULE_PATH}/uninstall.cmake"
)
