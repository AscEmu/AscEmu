# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(EXECUTABLE_NAME libmpq)

file(GLOB LIBMPQ_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/libmpq/*.c ${CMAKE_CURRENT_SOURCE_DIR}/libmpq/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/libmpq/*.h)

if (WIN32)
set(LIBMPQ_SRCS
    ${LIBMPQ_SRCS}
    win/winvs_config.h
    win/winvs_dirent.h
    win/winvs_stdint.h
)
endif ()

if (UNIX AND NOT IS_64BIT)
    add_definitions(-D_FILE_OFFSET_BITS=64)
endif ()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/win
    ${CMAKE_CURRENT_SOURCE_DIR}/libmpq
    ${CMAKE_SOURCE_DIR}/dep/loadlib
    ${BZIP2_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
)

add_library(${EXECUTABLE_NAME} STATIC ${LIBMPQ_SRCS})

# include library dependencies.
target_link_libraries(${EXECUTABLE_NAME}
  PUBLIC
    loadlib
    ${ZLIB_LIBRARIES}
    ${BZIP2_LIBRARIES}
)

unset(LIBMPQ_SRCS)
