# Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(LIBRARY_NAME libmpq)

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
    add_compile_options(-D_FILE_OFFSET_BITS=64)
endif ()

# source group.
ascemu_source_group(${CMAKE_CURRENT_SOURCE_DIR})

add_library(${LIBRARY_NAME} STATIC ${LIBMPQ_SRCS})

target_include_directories(${LIBRARY_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/win
    ${CMAKE_CURRENT_SOURCE_DIR}/libmpq
    ${CMAKE_SOURCE_DIR}/dep/loadlib
    ${BZIP2_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
)

# include library dependencies.
target_link_libraries(${LIBRARY_NAME}
  PUBLIC
    loadlib
    ${ZLIB_LIBRARIES}
    ${BZIP2_LIBRARIES}
)

unset(LIBMPQ_SRCS)
