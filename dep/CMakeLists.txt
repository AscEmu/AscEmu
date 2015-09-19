# add our dependecies
add_subdirectory( bzip2 )
add_subdirectory( libmpq_new )
add_subdirectory( dbcfile )
add_subdirectory( loadlib )

# add lualib if we need it for LuaEngin or LuaBridge
IF(BUILD_LUABRIDGE OR BUILD_LUAENGINE)
	add_subdirectory(lualib)
ENDIF()
