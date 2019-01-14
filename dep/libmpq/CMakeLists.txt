# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

project(libmpq C)
file(GLOB sources ${CMAKE_CURRENT_SOURCE_DIR}/libmpq/*.c ${CMAKE_CURRENT_SOURCE_DIR}/libmpq/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/libmpq/*.h)

if(WIN32)
    set(sources
    ${sources}
    win/winvs_config.h
    win/winvs_dirent.h
    win/winvs_stdint.h
    )
endif()

if(UNIX AND NOT IS_64BIT)
    add_definitions(-D_FILE_OFFSET_BITS=64)
endif()

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/win
   ${CMAKE_CURRENT_SOURCE_DIR}/libmpq
   ${CMAKE_SOURCE_DIR}/dep/loadlib
   ${BZIP2_INCLUDE_DIRS}
   ${ZLIB_INCLUDE_DIRS}
)

add_library(${PROJECT_NAME} STATIC ${sources})
target_link_libraries(${PROJECT_NAME} ${ZLIB_LIBRARIES} loadlib ${BZIP2_LIBRARIES})

unset(sources)
