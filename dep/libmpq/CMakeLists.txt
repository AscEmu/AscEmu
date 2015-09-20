file(GLOB libmpq_sources libmpq/*.c libmpq/*.h)

set(libmpq_SRCS
  ${libmpq_sources}
)

if(NOT WIN32 AND NOT IS_64BIT)
	add_definitions(-D_FILE_OFFSET_BITS=64)
endif(NOT WIN32 AND NOT IS_64BIT)

include_directories(libmpq ${DEPENDENCY_INCLUDE_ROOT}/zlib ${DEPENDENCY_INCLUDE_ROOT}/bzip2 ${DEPENDENCY_INCLUDE_ROOT}/loadlib)
add_library(libmpq STATIC ${libmpq_SRCS})
target_link_libraries(libmpq ${ZLIB_LIBRARIES} bzip2)
