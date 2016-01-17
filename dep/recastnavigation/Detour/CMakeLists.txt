# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

set(Detour_SRCS
   Source/DetourAlloc.cpp
   Source/DetourCommon.cpp
   Source/DetourNavMesh.cpp
   Source/DetourNavMeshBuilder.cpp
   Source/DetourNavMeshQuery.cpp
   Source/DetourNode.cpp
   )


include_directories(
   Include
   ${ZLIB_LIBRARIES}
   )

add_library(Detour STATIC ${Detour_SRCS})

#Set it to the project name.
set(DETOUR_LIBRARIES Detour CACHE INTERNAL "Detour Libraries")
unset(Detour_SRCS)