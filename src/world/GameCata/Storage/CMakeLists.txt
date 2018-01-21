# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX GameCata/Storage)

set(SRC_GAMECATA_STORAGE_FILES
    ${PATH_PREFIX}/DBCStores.cpp
    ${PATH_PREFIX}/DBCStores.h
    ${PATH_PREFIX}/DBCStructures.cpp
    ${PATH_PREFIX}/DBCStructures.h
)

source_group(GameCata\\Storage FILES ${SRC_GAMECATA_STORAGE_FILES})
unset(PATH_PREFIX)
