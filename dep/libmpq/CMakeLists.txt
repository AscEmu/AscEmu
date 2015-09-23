# Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>

file(GLOB libmpq_sources libmpq/*.c libmpq/*.h)
project(libmpq)

set(sources
   ${PROJECT_NAME}/common.c
   ${PROJECT_NAME}/explode.c
   ${PROJECT_NAME}/extract.c
   ${PROJECT_NAME}/huffman.c
   ${PROJECT_NAME}/mpq.c
   ${PROJECT_NAME}/mpq_libmpq.cpp
   ${PROJECT_NAME}/wave.c
   )

if(NOT WIN32 AND NOT IS_64BIT)
	add_definitions(-D_FILE_OFFSET_BITS=64)
endif(NOT WIN32 AND NOT IS_64BIT)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/libmpq
   ${CMAKE_CURRENT_SOURCE_DIR}/win
   ${DEPENDENCY_ROOT}/zlib
   ${DEPENDENCY_ROOT}/bzip2
   ${DEPENDENCY_ROOT}/loadlib
   )

add_library(${PROJECT_NAME} STATIC ${sources})
target_link_libraries(${PROJECT_NAME} ${ZLIB_LIBRARIES} bzip2)
