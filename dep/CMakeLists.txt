# Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>

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
    # mpqlib is the shared MPQ-reading layer for every version's tools now,
    # and also carries DBCFile/FileLoader (Classic/TBC/WotLK) plus the
    # BinaryReader/ChunkReader helpers they and vmap4_extractor build on -
    # libmpq itself (the old archive-format parser and its later thin
    # MPQArchive/MPQFile wrapper) and the separate dbcfile/loadlib libraries
    # are retired.
    add_subdirectory(mpqlib)
endif ()
