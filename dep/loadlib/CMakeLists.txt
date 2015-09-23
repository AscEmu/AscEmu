# Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>

# set up our project name
project(loadlib)

set(prefix ${DEPENDENCY_ROOT}/${PROJECT_NAME})

set(sources
   adt.cpp
   loadlib.cpp
   wdt.cpp
)

foreach(src IN ITEMS ${sources})
   set(SRCS ${SRCS} ${prefix}/${src})
endforeach(src)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${DEPENDENCY_ROOT}/libmpq/libmpq
   ${DEPENDENCY_ROOT}/libmpq/win
   )

add_library(${PROJECT_NAME} STATIC ${SRCS})
target_link_libraries(${PROJECT_NAME} libmpq)
