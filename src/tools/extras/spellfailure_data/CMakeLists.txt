# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(EXECUTABLE_NAME spellfailure_data)

file(GLOB SRC_SPELLFAILURE_DATA *.cpp)

set(SPELLFAILURE_DATA_SRCS
    ${SRC_SPELLFAILURE_DATA}
)

add_executable(${EXECUTABLE_NAME} ${SPELLFAILURE_DATA_SRCS})

# include library dependencies.
target_link_libraries(${EXECUTABLE_NAME}
  PUBLIC
    ${ZLIB_LIBRARIES}
)

install(TARGETS ${EXECUTABLE_NAME} RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})
