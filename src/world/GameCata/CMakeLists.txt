# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

include(GameCata/Management/CMakeLists.txt)
include(GameCata/Movement/CMakeLists.txt)
include(GameCata/Network/CMakeLists.txt)
include(GameCata/Storage/CMakeLists.txt)

set(PATH_PREFIX GameCata)

set(SRC_GAMECATA_FILES
    ${PATH_PREFIX}/UpdateFields.h
)

source_group(GameCata FILES ${SRC_GAMECATA_FILES})
unset(PATH_PREFIX)
