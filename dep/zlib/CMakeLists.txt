# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

# set up our project name
project(zlib C)
set(sources
   adler32.c
   compress.c
   crc32.c
   deflate.c
   infback.c
   inffast.c
   inflate.c
   inftrees.c
   trees.c
   uncompr.c
   zutil.c
   )

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(${PROJECT_NAME} STATIC ${sources} )

#set it to the project name.
set(ZLIB_LIBRARIES ${PROJECT_NAME} CACHE INTERNAL "Zlib Libraries")
set(ZLIB_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "Zlib include dir" FORCE)
unset(sources)