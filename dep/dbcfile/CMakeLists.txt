# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(LIBRARY_NAME dbcfile)

set(SRC_DBCFILE
    dbcfile.cpp
)

add_library(${LIBRARY_NAME} STATIC ${SRC_DBCFILE})

target_include_directories(${LIBRARY_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/dep/loadlib
    ${CMAKE_SOURCE_DIR}/dep/libmpq/libmpq
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# include library dependencies.
target_link_libraries(${LIBRARY_NAME}
  PUBLIC
    loadlib
    libmpq
)

unset(SRC_DBCFILE)
