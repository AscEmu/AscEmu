# Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>

# set up our project name
project(Recast)

set(prefix ${RECASTNAVIGATION}/${PROJECT_NAME}/source)

set(sources
   Recast.cpp
   RecastAlloc.cpp
   RecastArea.cpp
   RecastContour.cpp
   RecastFilter.cpp
   RecastMesh.cpp
   RecastMeshDetail.cpp
   RecastRasterization.cpp
   RecastRegion.cpp
   )

set(headers
   Recast.h
   RecastAlloc.h
   RecastAssert.h
   )

foreach(src IN ITEMS ${sources})
   set(SRCS ${SRCS} ${prefix}/${src})
endforeach(src)
foreach(src IN ITEMS ${headers})
   set(HEADERS ${HEADERS} ${prefix}/${src})
endforeach(src)

include_directories(${GLOBAL_INCLUDE_DIRS})
add_library(${PROJECT_NAME} STATIC ${SRCS})

#Set it to the project name.
set(RECAST_LIBRARIES ${PROJECT_NAME} CACHE INTERNAL "recast Libraries")
