# Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>

# set up our project name
project(bzip2)

set(prefix ${DEPENDENCY_ROOT}/${PROJECT_NAME})

set(sources
   blocksort.c
   bzlib.c
   compress.c
   crctable.c
   decompress.c
   huffman.c
   randtable.c
   )

foreach(src IN ITEMS ${sources})
   set(SRCS ${SRCS} ${prefix}/${src})
endforeach(src)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(${PROJECT_NAME} STATIC ${SRCS})
target_link_libraries(${PROJECT_NAME})
