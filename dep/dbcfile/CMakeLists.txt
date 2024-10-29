# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(EXECUTABLE_NAME dbcfile)

set(SRC_DBCFILE
    dbcfile.cpp
)

include_directories(
    ${CMAKE_SOURCE_DIR}/dep/loadlib
    ${CMAKE_SOURCE_DIR}/dep/libmpq/libmpq
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(${EXECUTABLE_NAME} STATIC ${SRC_DBCFILE})

# include library dependencies.
target_link_libraries(${EXECUTABLE_NAME}
  PUBLIC
    loadlib
    libmpq
)

unset(SRC_DBCFILE)
