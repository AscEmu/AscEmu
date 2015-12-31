# Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>

if(CMAKE_GENERATOR MATCHES Unix*)
	add_definitions(-Wno-deprecated)
	#Extract Revision
	execute_process(WORKING_DIRECTORY ${ROOT_PATH} COMMAND sh git_version.sh)
	
	#Check support for unordered_map/set
	GetCompilerVersion( gcc_version)
	if(DEFINED gcc_version AND (${gcc_version} VERSION_EQUAL "4.3" OR ${gcc_version} VERSION_GREATER "4.3" ) AND NOT APPLE)
		add_definitions(-DHAS_CXX0X -std=gnu++0x)
	endif()
	
elseif(CMAKE_GENERATOR MATCHES Visual*)
	#Extract revision
	execute_process(WORKING_DIRECTORY ${ROOT_PATH} COMMAND git_version.bat)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS /EHa)
	
	#Check support for unordered_map/set
	GetCompilerVersion(msvc_version)
	if(DEFINED ${msvc_version} AND ${msvc_version} GREATER "1599")
		add_definitions(-DHAS_CXX0X)
	endif()
	
	#This fixes PCH issues 'Inconsistent values for /Zm'
	if(${CMAKE_CXX_FLAGS} MATCHES "(/Zm)([0-9]+)")
		string(REGEX REPLACE "(/Zm)([0-9]+)" "\\1${VISUALSTUDIO_COMPILERHEAPLIMIT}" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zm${VISUALSTUDIO_COMPILERHEAPLIMIT}")
	endif()

	if(${CMAKE_C_FLAGS} MATCHES "(/Zm)([0-9]+)")
		string(REGEX REPLACE "(/Zm)([0-9]+)" "\\1${VISUALSTUDIO_COMPILERHEAPLIMIT}" CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
	else()
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Zm${VISUALSTUDIO_COMPILERHEAPLIMIT}")
	endif()
		
endif()
