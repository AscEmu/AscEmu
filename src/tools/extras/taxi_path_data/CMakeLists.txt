# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

file(GLOB taxi_path_data_sources *.cpp *.h)

set(taxi_path_SRCS
  ${taxi_path_data_sources}
)

if(NOT WIN32 AND NOT IS_64BIT)
    add_definitions(-D_FILE_OFFSET_BITS=64)
endif()

include_directories(
   ${LOADLIB_PATH}
   )

add_executable(taxi_path_data ${taxi_path_SRCS})
target_link_libraries(taxi_path_data ${ZLIB_LIBRARIES})
install(TARGETS taxi_path_data RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})
