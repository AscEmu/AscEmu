# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(EXECUTABLE_NAME taxi_path_data)

file(GLOB SRC_TAXI_PATH_DATA *.cpp *.h)

add_executable(${EXECUTABLE_NAME} ${SRC_TAXI_PATH_DATA})

if (NOT WIN32 AND NOT IS_64BIT)
    add_compile_options(-D_FILE_OFFSET_BITS=64)
endif ()

include_directories(
    ${LOADLIB_PATH}
)

# include library dependencies.
target_link_libraries(${EXECUTABLE_NAME}
  PUBLIC
    ${ZLIB_LIBRARIES}
)

install(TARGETS ${EXECUTABLE_NAME} RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})
