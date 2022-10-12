# Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Management/Battleground)

set(SRC_MANAGEMENT_BATTLEGROUND_FILES
    ${PATH_PREFIX}/Battleground.cpp
    ${PATH_PREFIX}/Battleground.hpp
	${PATH_PREFIX}/BattlegroundDefines.hpp
    ${PATH_PREFIX}/BattlegroundMgr.cpp
    ${PATH_PREFIX}/BattlegroundMgr.hpp
)

source_group(Management\\Battleground FILES ${SRC_MANAGEMENT_BATTLEGROUND_FILES})
unset(PATH_PREFIX)
