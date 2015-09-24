# Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>

set(Recast_SRCS
   Source/Recast.cpp
   Source/RecastAlloc.cpp
   Source/RecastArea.cpp
   Source/RecastContour.cpp
   Source/RecastFilter.cpp
   Source/RecastMesh.cpp
   Source/RecastMeshDetail.cpp
   Source/RecastRasterization.cpp
   Source/RecastRegion.cpp
   )

include_directories(
   Include
   ${ZLIB_LIBRARIES}
   )

add_library(Recast STATIC ${Recast_SRCS})

#Set it to the project name.
set(RECAST_LIBRARIES Recast CACHE INTERNAL "recast Libraries")
