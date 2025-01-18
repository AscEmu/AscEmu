# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(LIBRARY_NAME zlib)

set(SRC_ZLIB
    adler32.c
    compress.c
    crc32.c
    deflate.c
    infback.c
    inffast.c
    gzclose.c
    gzlib.c
    gzread.c
    gzwrite.c
    inflate.c
    infback.c
    inftrees.c
    inffast.c
    trees.c
    uncompr.c
    zutil.c
)

# source group.
ascemu_source_group(${CMAKE_CURRENT_SOURCE_DIR})

add_library(${LIBRARY_NAME} STATIC ${SRC_ZLIB})

target_include_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#set it to the project name.
set(ZLIB_LIBRARIES ${LIBRARY_NAME} CACHE INTERNAL "Zlib Libraries")
set(ZLIB_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "Zlib include dir" FORCE)

unset(SRC_ZLIB)
