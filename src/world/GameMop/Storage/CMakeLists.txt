# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX GameMop/Storage)

set(SRC_GAMEMOP_STORAGE_FILES
    ${PATH_PREFIX}/DBCStores.cpp
    ${PATH_PREFIX}/DBCStores.h
    ${PATH_PREFIX}/DBCStructures.cpp
    ${PATH_PREFIX}/DBCStructures.h
)

source_group(GameMop\\Storage FILES ${SRC_GAMEMOP_STORAGE_FILES})
unset(PATH_PREFIX)
