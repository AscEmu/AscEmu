# add crashreport tool if we are not on windows
IF(NOT WIN32)
	add_subdirectory(crashreport)
ENDIF()

# add our executable related folders
add_subdirectory(shared)
add_subdirectory(logonserver)
add_subdirectory(world)

# if build tools is set, add the subdirectory
IF( BUILD_TOOLS )
	add_subdirectory(tools)
ENDIF( BUILD_TOOLS )