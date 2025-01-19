# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>

#ignore the warnings for dependencies (external code)
if (MSVC)
    add_compile_options(/W0)
else ()
    add_compile_options(-w)
endif ()

# add zlib and bzip2 for windows compile
if (WIN32)
    add_subdirectory(bzip2)
    add_subdirectory(zlib)
endif ()

# add our dependecies
add_subdirectory(recastnavigation)
add_subdirectory(g3dlite)
add_subdirectory(utf8cpp)
add_subdirectory(openssl)
add_subdirectory(fmt)

if (BUILD_ASCEMUSCRIPTS AND BUILD_LUAENGINE)
    add_subdirectory(lualib/src)
endif ()

if (BUILD_TOOLS)
    if ("${ASCEMU_VERSION}" STREQUAL "Cata" OR "${ASCEMU_VERSION}" STREQUAL "Mop")
        add_subdirectory(StormLib)
    else ()
        add_subdirectory(libmpq)
        add_subdirectory(dbcfile)
        add_subdirectory(loadlib)
    endif ()
endif ()
