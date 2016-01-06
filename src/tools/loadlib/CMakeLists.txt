# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

# set up our project name
file(GLOB loadlib_source *.h)

set(loadlib_SRCS
   loadlib_source
)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${DEPENDENCY_ROOT}/libmpq/libmpq
   ${DEPENDENCY_ROOT}/libmpq/win
   )

add_library(${loadlib_SRCS} STATIC ${SRCS})
target_link_libraries(${loadlib_SRCS} libmpq)
