if( CMAKE_GENERATOR MATCHES Unix* )
	add_definitions(-Wno-deprecated)
	#Extract Revision
	execute_process(WORKING_DIRECTORY ${ROOT_PATH} COMMAND sh git_version.sh )
	
	#Check support for unordered_map/set
	GetCompilerVersion( gcc_version)
	IF( DEFINED gcc_version AND ( ${gcc_version} VERSION_EQUAL "4.3" OR ${gcc_version} VERSION_GREATER "4.3" ) AND NOT APPLE )
		add_definitions(-DHAS_CXX0X -std=gnu++0x)
	ENDIF()
	
ELSEIF( CMAKE_GENERATOR MATCHES Visual* )
	#Extract revision
	execute_process(WORKING_DIRECTORY ${ROOT_PATH} COMMAND git_version.bat )
	add_definitions(-D_CRT_SECURE_NO_WARNINGS /EHa )
	
	#Check support for unordered_map/set
	GetCompilerVersion( msvc_version)
	IF(DEFINED ${msvc_version} AND ${msvc_version} GREATER "1599" )
		add_definitions(-DHAS_CXX0X)
	ENDIF()
	
	#This fixes PCH issues 'Inconsistent values for /Zm'
	IF( ${CMAKE_CXX_FLAGS} MATCHES "(/Zm)([0-9]+)" )
		STRING( REGEX REPLACE "(/Zm)([0-9]+)" "\\1${VISUALSTUDIO_COMPILERHEAPLIMIT}" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} )
	ELSE()
		SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zm${VISUALSTUDIO_COMPILERHEAPLIMIT}"  )
	ENDIF()
		
	IF( ${CMAKE_C_FLAGS} MATCHES "(/Zm)([0-9]+)" )
		STRING( REGEX REPLACE "(/Zm)([0-9]+)" "\\1${VISUALSTUDIO_COMPILERHEAPLIMIT}" CMAKE_C_FLAGS ${CMAKE_C_FLAGS} )
	ELSE()
		SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Zm${VISUALSTUDIO_COMPILERHEAPLIMIT}"  )
	ENDIF()
		
ENDIF()