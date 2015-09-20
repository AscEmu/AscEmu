PROJECT(creature_data CXX)
SET( prefix ${ROOT_PATH}/src/tools/creature_data)
SET( sources
	Main.cpp
)

SET( headers
	structs.h
)
foreach(src IN ITEMS ${sources} )
  SET( SRCS ${SRCS} ${prefix}/${src} )
endforeach(src)
foreach(src IN ITEMS ${headers} )
  SET( HEADERS ${HEADERS} ${prefix}/${src} )
endforeach(src)

IF( NOT WIN32 AND NOT IS_64BIT )
	add_definitions( -D_FILE_OFFSET_BITS=64 )
ENDIF( NOT WIN32 AND NOT IS_64BIT )

include_directories(
   ${DEPENDENCY_ROOT}/libmpq_new
   ${DEPENDENCY_ROOT}/dbcfile
   ${DEPENDENCY_ROOT}/loadlib
   )

ADD_EXECUTABLE( ${PROJECT_NAME} ${SRCS} ${HEADERS} )

target_link_libraries( ${PROJECT_NAME} dbcfile ${ZLIB_LIBRARIES})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH} )

