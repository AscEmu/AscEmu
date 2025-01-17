# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(LIBRARY_NAME loadlib)

file(GLOB SRC_LOADLIB *cpp *.h)

set(LOADLIB_SRCS
    ${SRC_LOADLIB}
)

add_library(${LIBRARY_NAME} STATIC ${LOADLIB_SRCS})

target_include_directories(${LIBRARY_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/dep/libmpq/libmpq
    ${CMAKE_SOURCE_DIR}/dep/libmpq/win
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# include library dependencies.
target_link_libraries(${LIBRARY_NAME}
  PUBLIC
    libmpq
)

unset(LOADLIB_SRCS)
