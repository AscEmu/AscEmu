# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

# set up our project name
project(bzip2 C)

set(sources
   blocksort.c
   bzlib.c
   compress.c
   crctable.c
   decompress.c
   huffman.c
   randtable.c
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(${PROJECT_NAME} STATIC ${sources})
target_link_libraries(${PROJECT_NAME})
set(BZIP2_LIBRARIES ${PROJECT_NAME} CACHE INTERNAL "Bzip2 libraries." FORCE)
set(BZIP2_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "Bzip2 include dirs." FORCE)

unset(sources)