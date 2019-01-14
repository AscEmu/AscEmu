# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

include(GameMop/Management/CMakeLists.txt)
include(GameMop/Movement/CMakeLists.txt)
include(GameMop/Network/CMakeLists.txt)
include(GameMop/Storage/CMakeLists.txt)

set(PATH_PREFIX GameMop)

set(SRC_GAMEMOP_FILES
    ${PATH_PREFIX}/UpdateFields.h
)

source_group(GameMop FILES ${SRC_GAMEMOP_FILES})
unset(PATH_PREFIX)
