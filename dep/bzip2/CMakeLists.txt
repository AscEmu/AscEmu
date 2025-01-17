# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(LIBRARY_NAME bzip2)

set(SRC_BZIP2
    blocksort.c
    bzlib.c
    compress.c
    crctable.c
    decompress.c
    huffman.c
    randtable.c
)

add_library(${LIBRARY_NAME} STATIC ${SRC_BZIP2})

target_include_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set(BZIP2_LIBRARIES ${LIBRARY_NAME} CACHE INTERNAL "Bzip2 libraries." FORCE)
set(BZIP2_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "Bzip2 include dirs." FORCE)

unset(SRC_BZIP2)
