# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

# set up project name
project(vmap4_extractor CXX)
file(GLOB source *.cpp *.h)

set(source
   ${source}
)

if(UNIX AND NOT IS_64BIT)
    add_definitions(-D_FILE_OFFSET_BITS=64)
endif()

include_directories(
   ${CMAKE_SOURCE_DIR}/dep/loadlib
   ${CMAKE_SOURCE_DIR}/dep/dbcfile 
   ${CMAKE_SOURCE_DIR}/dep/libmpq/libmpq
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${BZIP2_INCLUDE_DIRS}
   ${ZLIB_INCLUDE_DIRS}
)

add_executable(${PROJECT_NAME} ${source})
target_link_libraries(${PROJECT_NAME} dbcfile loadlib ${ZLIB_LIBRARIES} ${BZIP2_LIBRARIES})
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})

unset(sources)
unset(headers)
