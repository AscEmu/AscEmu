# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
project(vmap4_extractor CXX)

file(GLOB SRC_VMAPS_EXTRACTOR *.cpp *.h)

add_executable(${PROJECT_NAME} ${SRC_VMAPS_EXTRACTOR})

if (UNIX AND NOT IS_64BIT)
    add_definitions(-D_FILE_OFFSET_BITS=64)
endif ()

include_directories(
    ${CMAKE_SOURCE_DIR}/dep/loadlib
    ${CMAKE_SOURCE_DIR}/dep/dbcfile 
    ${CMAKE_SOURCE_DIR}/dep/libmpq/libmpq
    ${CMAKE_SOURCE_DIR}/src/shared
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${BZIP2_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
)

# include library dependencies.
target_link_libraries(${PROJECT_NAME}
  PUBLIC
    dbcfile
    loadlib
    ${ZLIB_LIBRARIES}
    ${BZIP2_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})
