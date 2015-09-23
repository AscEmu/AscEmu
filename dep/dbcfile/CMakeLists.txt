# Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>

# set up our project name
project(dbcfile)

set(prefix ${DEPENDENCY_ROOT}/${PROJECT_NAME})

set(sources
   dbcfile.cpp
   )

foreach(src IN ITEMS ${sources})
   set(SRCS ${SRCS} ${prefix}/${src})
endforeach(src)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${DEPENDENCY_ROOT}/loadlib
   ${DEPENDENCY_ROOT}/libmpq/libmpq
   )

link_directories(${DEPENDENCY_LIBS} ${EXTRA_LIBS_PATH})
add_library(${PROJECT_NAME} STATIC ${SRCS})
target_link_libraries( ${PROJECT_NAME} libmpq)
