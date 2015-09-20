PROJECT(map_extractor CXX)
SET( prefix ${ROOT_PATH}/src/tools/map_extractor)
SET( sources
	Map.cpp
)
SET( headers
   Map.h
)
foreach(src IN ITEMS ${sources} )
  SET( SRCS ${SRCS} ${prefix}/${src} )
endforeach(src)
foreach(src IN ITEMS ${headers} )
  SET( HEADERS ${HEADERS} ${prefix}/${src} )
endforeach(src)

include_directories(
  ${DEPENDENCY_INCLUDE_ROOT}/libmpq_new
  ${DEPENDENCY_ROOT}/dbcfile 
  ${DEPENDENCY_ROOT}/loadlib )

ADD_EXECUTABLE( ${PROJECT_NAME} ${SRCS} )

target_link_libraries( ${PROJECT_NAME} dbcfile loadlib ${ZLIB_LIBRARIES} )

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH} )

