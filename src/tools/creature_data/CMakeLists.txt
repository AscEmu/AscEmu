# Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>

file(GLOB creature_data_sources *.cpp *.h)
set(creature_SRCS
  ${creature_data_sources}
)

if(NOT WIN32 AND NOT IS_64BIT)
	add_definitions(-D_FILE_OFFSET_BITS=64)
endif()

include_directories(
   ${DEPENDENCY_ROOT}/libmpq_new
   ${DEPENDENCY_ROOT}/dbcfile
   ${DEPENDENCY_ROOT}/loadlib
   )

add_executable(creature_data ${creature_SRCS})
target_link_libraries(creature_data dbcfile ${ZLIB_LIBRARIES})
install(TARGETS creature_data RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})
