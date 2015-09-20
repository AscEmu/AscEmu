# Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>

# set up our project name
project(libmpq_new)

set(prefix ${DEPENDENCY_ROOT}/${PROJECT_NAME})

set(sources
   common.c
   explode.c
   extract.c
   huffman.c
   mpq.c
   mpq_libmpq.cpp
   wave.c
   )

foreach(src IN ITEMS ${sources})
   set(SRCS ${SRCS} ${prefix}/${src})
endforeach(src)

if(NOT WIN32 AND NOT IS_64BIT)
	add_definitions(-D_FILE_OFFSET_BITS=64)
endif(NOT WIN32 AND NOT IS_64BIT)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${DEPENDENCY_ROOT}/zlib ${DEPENDENCY_ROOT}/bzip2 ${DEPENDENCY_ROOT}/loadlib)
add_library(${PROJECT_NAME} STATIC ${SRCS})
target_link_libraries(${PROJECT_NAME} ${ZLIB_LIBRARIES} bzip2)
