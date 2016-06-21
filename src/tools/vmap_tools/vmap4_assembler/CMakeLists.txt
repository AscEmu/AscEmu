# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

# set up project name
project(vmap4_assembler CXX)

set(sources
   vmap_assembler.cpp
)

include_directories(
   ${CMAKE_SOURCE_DIR}/dep/vmaplib/include
   ${CMAKE_SOURCE_DIR}/dep/g3dlite/include
   ${ZLIB_INCLUDE_DIRS}
)

add_executable(${PROJECT_NAME} ${sources})
target_link_libraries(${PROJECT_NAME} vmaplib g3dlite ${ZLIB_LIBRARIES})
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})

unset(sources)