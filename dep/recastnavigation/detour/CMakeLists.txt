# Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>

# set up our project name
project(detour)

set(prefix ${RECASTNAVIGATION}/${PROJECT_NAME}/source)
set(hprefix ${RECASTNAVIGATION}/${PROJECT_NAME}/include)

set(sources
   DetourAlloc.cpp
   DetourCommon.cpp
   DetourNavMesh.cpp
   DetourNavMeshBuilder.cpp
   DetourNavMeshQuery.cpp
   DetourNode.cpp
   DetourObstacleAvoidance.cpp
   )

set(headers
   DetourAlloc.h
   DetourAssert.h
   DetourCommon.h
   DetourNavMesh.h
   DetourNavMeshBuilder.h
   DetourNavMeshQuery.h
   DetourNode.h
   DetourObstacleAvoidance.h
   )

foreach(src IN ITEMS ${sources})
   set(SRCS ${SRCS} ${prefix}/${src})
endforeach(src)
foreach(src IN ITEMS ${headers})
   set(HEADERS ${HEADERS} ${hprefix}/${src})
endforeach(src)


include_directories(${GLOBAL_INCLUDE_DIRS})
add_library(${PROJECT_NAME} STATIC ${SRCS} ${HEADERS})

#Set it to the project name.
set(DETOUR_LIBRARIES ${PROJECT_NAME} CACHE INTERNAL "Detour Libraries")
