# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

# set up our project name.
project(vmap4_assembler CXX)

file(GLOB SRC_VMAPS_EXTRACTOR *.cpp *.h)

set(VMAPS_EXTRACTOR_SRCS
    ${SRC_VMAPS_EXTRACTOR}
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

add_executable(${PROJECT_NAME} ${VMAPS_EXTRACTOR_SRCS})

# include library dependencies.
target_link_libraries(${PROJECT_NAME}
  PUBLIC
    collision
    g3dlite
    ${ZLIB_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})
