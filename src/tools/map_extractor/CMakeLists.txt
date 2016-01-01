# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

file(GLOB map_extractor_source *.cpp *.h)
set(map_extractor_SRCS
  ${map_extractor_source}
)

include_directories(
  ${DEPENDENCY_ROOT}/libmpq/libmpq
  ${DEPENDENCY_ROOT}/libmpq/win
  ${DEPENDENCY_ROOT}/dbcfile 
  ${LOADLIB_PATH}
  )

add_executable(map_extractor ${map_extractor_SRCS})
target_link_libraries(map_extractor dbcfile ${ZLIB_LIBRARIES})
install(TARGETS map_extractor RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})
