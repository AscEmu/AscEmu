PROJECT(mmaps_generator CXX)
SET( prefix ${ROOT_PATH}/src/tools/mmaps_generator)
SET( sources
    generator.cpp
	IntermediateValues.cpp
	MapBuilder.cpp
	TerrainBuilder.cpp
	VMapExtensions.cpp
)

SET( headers
   IntermediateValues.h
   MoveMapSharedDefines.h
   MapBuilder.h
   TerrainBuilder.h
   CompilerDefs.h
   Define.h
)

foreach(src IN ITEMS ${sources} )
  SET( SRCS ${SRCS} ${prefix}/${src} )
endforeach(src)
foreach(src IN ITEMS ${headers} )
  SET( HEADERS ${HEADERS} ${prefix}/${src} )
endforeach(src)

include_directories(
  ${DEPENDENCY_ROOT}/libmpq_new
  ${DEPENDENCY_ROOT}/dbcfile 
  ${DEPENDENCY_ROOT}/loadlib
  ${DEPENDENCY_ROOT}/vmaplib
  ${RECASTNAVIGATION}/detour/include
  ${RECASTNAVIGATION}/recast/include
  ${DEPENDENCY_ROOT}/g3dlite/include
  )
link_directories( ${DEPENDENCY_LIBS} ${EXTRA_LIBS_PATH} )

ADD_EXECUTABLE( ${PROJECT_NAME} ${SRCS}  ${HEADERS} )

target_link_libraries( ${PROJECT_NAME} shared  g3dlite vmaplib detour recast ${PCRE_LIBRARIES} )

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH} )

