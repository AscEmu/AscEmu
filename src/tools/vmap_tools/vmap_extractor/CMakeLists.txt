# Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>

# set up project name
project(vmap_extractor CXX)

set(prefix ${ROOT_PATH}/src/tools/vmap_tools/vmap_extractor)

set(sources
   adtfile.cpp
   dbcfile.cpp
   model.cpp
   vmapexport.cpp
   wdtfile.cpp
   wmo.cpp
   )

set(headers
   adtfile.h
   dbcfile.h
   model.h
   modelheaders.h
   vec3d.h
   vmapexport.h
   wdtfile.h
   wmo.h
   )

foreach(src IN ITEMS ${sources})
   set(SRCS ${SRCS} ${prefix}/${src})
endforeach(src)
foreach(src IN ITEMS ${headers})
   set(HEADERS ${HEADERS} ${prefix}/${src})
endforeach(src)

if(NOT WIN32 AND NOT IS_64BIT)
	add_definitions(-D_FILE_OFFSET_BITS=64)
endif(NOT WIN32 AND NOT IS_64BIT)

include_directories(
   ${DEPENDENCY_ROOT}/libmpq_new
   ${DEPENDENCY_ROOT}/loadlib
   ${DEPENDENCY_INCLUDE_ROOT}/zlib
   ${DEPENDENCY_INCLUDE_ROOT}/bzip2
   )

add_executable(${PROJECT_NAME} ${SRCS} ${HEADERS})
target_link_libraries(${PROJECT_NAME} libmpq_new loadlib ${ZLIB_LIBRARIES} bzip2)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})
