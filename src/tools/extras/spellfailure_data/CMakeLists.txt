# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

file(GLOB spellfailure_data_sources *.cpp)

set(spellfailure_SRCS
  ${spellfailure_data_sources}
)

add_executable(spellfailure_data ${spellfailure_SRCS})
target_link_libraries(spellfailure_data ${ZLIB_LIBRARIES})
install(TARGETS spellfailure_data RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})
