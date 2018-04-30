# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

# set up our project name
project(loadlib CXX)
file(GLOB loadlib_source *cpp *.h)

include_directories(
   ${CMAKE_SOURCE_DIR}/dep/libmpq/libmpq
   ${CMAKE_SOURCE_DIR}/dep/libmpq/win
   ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(${PROJECT_NAME} STATIC ${loadlib_source})
target_link_libraries(${PROJECT_NAME} libmpq)

unset(loadlib_source)