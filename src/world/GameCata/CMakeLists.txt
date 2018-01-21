# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

include(GameCata/Handlers/CMakeLists.txt)
include(GameCata/Management/CMakeLists.txt)
include(GameCata/Movement/CMakeLists.txt)
include(GameCata/Network/CMakeLists.txt)
include(GameCata/Storage/CMakeLists.txt)
include(GameCata/Units/CMakeLists.txt)
include(GameCata/Spell/CMakeLists.txt)

set(PATH_PREFIX GameCata)

set(SRC_GAMECATA_FILES
    ${PATH_PREFIX}/UpdateFields.h
)

source_group(GameCata FILES ${SRC_GAMECATA_FILES})
unset(PATH_PREFIX)
