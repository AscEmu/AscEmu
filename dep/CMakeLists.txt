# add our dependecies
add_subdirectory(bzip2)
add_subdirectory(dbcfile)
add_subdirectory(loadlib)
add_subdirectory(vmaplib)
add_subdirectory(detour)
add_subdirectory(recast)
add_subdirectory(g3dlite)
add_subdirectory(libmpq_new)

# add zlib and pcre for windows compile
IF(NEED_ZLIB_PCRE)
	add_subdirectory(zlib)
	add_subdirectory(pcre)
ENDIF()

# add lualib if we need it for LuaEngin or LuaBridge
IF(BUILD_LUABRIDGE OR BUILD_LUAENGINE)
	add_subdirectory(lualib)
ENDIF()
