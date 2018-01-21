# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

# set up project name
project(vmap4_assembler CXX)

set(sources
   VMapAssembler.cpp
)

include_directories(
   ${CMAKE_SOURCE_DIR}/src/shared
   ${CMAKE_SOURCE_DIR}/src/collision
   ${CMAKE_SOURCE_DIR}/src/collision/Management
   ${CMAKE_SOURCE_DIR}/src/collision/Maps
   ${CMAKE_SOURCE_DIR}/src/collision/Models
   ${CMAKE_SOURCE_DIR}/dep/g3dlite/include
   ${ZLIB_INCLUDE_DIRS}
)

add_executable(${PROJECT_NAME} ${sources})
target_link_libraries(${PROJECT_NAME} collision g3dlite ${ZLIB_LIBRARIES})
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})

unset(sources)