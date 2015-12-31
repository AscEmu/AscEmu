# Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>

macro(GetCompilerVersion out_version)
	#Test for gnu compilers
	if(CMAKE_COMPILER_IS_GNUCXX)
		execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE tmp_version)
		set(${out_version} ${tmp_version})
	elseif(CMAKE_COMPILER_IS_GNUC)
		execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE tmp_version)
		set(${out_version} ${tmp_version})
	elseif(MSVC)
		set(${out_version} ${MSVC_VERSION})
	else()
		message(FATAL_ERROR "This function does not support the current compiler!")
	endif()
endmacro(GetCompilerVersion)
