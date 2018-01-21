# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

#ignore the warnings for dependencies (external code)
if(MSVC)
    add_definitions(/W0)
else()
    add_definitions(-w)
endif()

# add zlib and pcre for windows compile
if(WIN32)
    add_subdirectory(bzip2)
    add_subdirectory(zlib)
endif()

# add our dependecies
add_subdirectory(recastnavigation)
add_subdirectory(g3dlite)

add_subdirectory(lualib/src)

if(BUILD_TOOLS)
    if("${ASCEMU_VERSION}" STREQUAL "Cata")
        add_subdirectory(StormLib)
    else()
        add_subdirectory(libmpq)
        add_subdirectory(dbcfile)
        add_subdirectory(loadlib)
    endif()
endif()
