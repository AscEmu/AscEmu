# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(EXECUTABLE_NAME vmap4_assembler)

file(GLOB SRC_VMAPS_EXTRACTOR *.cpp *.h)

add_executable(${EXECUTABLE_NAME} ${SRC_VMAPS_EXTRACTOR})

include_directories(
    ${CMAKE_SOURCE_DIR}/src/shared
    ${CMAKE_SOURCE_DIR}/src/collision
    ${CMAKE_SOURCE_DIR}/src/collision/Management
    ${CMAKE_SOURCE_DIR}/src/collision/Maps
    ${CMAKE_SOURCE_DIR}/src/collision/Models
    ${CMAKE_SOURCE_DIR}/dep/g3dlite/include
    ${ZLIB_INCLUDE_DIRS}
)

# include library dependencies.
target_link_libraries(${EXECUTABLE_NAME}
  PUBLIC
    collision
    g3dlite
    ${ZLIB_LIBRARIES}
)

install(TARGETS ${EXECUTABLE_NAME} RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})
