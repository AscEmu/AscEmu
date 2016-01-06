# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

# set up project name
project(vmap_assembler CXX)

set(prefix ${ROOT_PATH}/src/tools/vmap_tools/vmap_assembler)

set(sources
   vmap_assembler.cpp
   )

foreach(src IN ITEMS ${sources})
   set(SRCS ${SRCS} ${prefix}/${src})
endforeach(src)

include_directories(
   ${DEPENDENCY_ROOT}/vmaplib/include
   ${DEPENDENCY_ROOT}/g3dlite/include
   )

add_executable(${PROJECT_NAME} ${SRCS})

if(APPLE)
	target_link_libraries(${PROJECT_NAME} vmaplib g3dlite ${ZLIB_LIBRARIES} "-framework Carbon")
else()
	target_link_libraries(${PROJECT_NAME} vmaplib g3dlite ${ZLIB_LIBRARIES})
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})
