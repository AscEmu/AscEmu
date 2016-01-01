# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

# set up our project name
project(zlib C)

set(prefix ${DEPENDENCY_ROOT}/${PROJECT_NAME})

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

foreach(src IN ITEMS ${sources})
   set(SRCS ${SRCS} ${prefix}/${src})
endforeach(src)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(${PROJECT_NAME} STATIC ${SRCS} )

#set it to the project name.
set(ZLIB_LIBRARIES ${PROJECT_NAME} CACHE INTERNAL "Zlib Libraries")
