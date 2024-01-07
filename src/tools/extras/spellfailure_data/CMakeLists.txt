# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
project(spellfailure_data CXX)

file(GLOB SRC_SPELLFAILURE_DATA *.cpp)

set(SPELLFAILURE_DATA_SRCS
    ${SRC_SPELLFAILURE_DATA}
)

add_executable(${PROJECT_NAME} ${SPELLFAILURE_DATA_SRCS})

# include library dependencies.
target_link_libraries(${PROJECT_NAME}
  PUBLIC
    ${ZLIB_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})
