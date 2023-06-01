# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

# set up our project name.
project(dbcfile CXX)

set(SRC_DBCFILE
    dbcfile.cpp
)

include_directories(
    ${CMAKE_SOURCE_DIR}/dep/loadlib
    ${CMAKE_SOURCE_DIR}/dep/libmpq/libmpq
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(${PROJECT_NAME} STATIC ${SRC_DBCFILE})

# include library dependencies.
target_link_libraries(${PROJECT_NAME}
  PUBLIC
    loadlib
    libmpq
)
