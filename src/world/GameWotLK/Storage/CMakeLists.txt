# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX GameWotLK/Storage)

set(SRC_GAMELK_STORAGE_FILES
    ${PATH_PREFIX}/DBCStores.cpp
    ${PATH_PREFIX}/DBCStores.h
    ${PATH_PREFIX}/DBCStructures.h
)

source_group(GameWotLK\\Storage FILES ${SRC_GAMELK_STORAGE_FILES})
unset(PATH_PREFIX)
