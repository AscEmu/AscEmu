# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

# set up project name
project(vmap_extractor CXX)

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

if(UNIX AND NOT IS_64BIT)
	add_definitions(-D_FILE_OFFSET_BITS=64)
endif()

include_directories(
   ${CMAKE_SOURCE_DIR}/dep/loadlib
   ${CMAKE_SOURCE_DIR}/dep/libmpq/libmpq
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${BZIP2_INCLUDE_DIRS}
   ${ZLIB_INCLUDE_DIRS}
)

add_executable(${PROJECT_NAME} ${sources} ${headers})
target_link_libraries(${PROJECT_NAME} loadlib ${ZLIB_LIBRARIES} ${BZIP2_LIBRARIES})
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})

unset(sources)
unset(headers)