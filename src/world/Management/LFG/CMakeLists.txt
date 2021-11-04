# Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Management/LFG)

set(SRC_MANAGEMENT_LFG_FILES
    ${PATH_PREFIX}/LFG.hpp
    ${PATH_PREFIX}/LFGGroupData.cpp
    ${PATH_PREFIX}/LFGGroupData.hpp
    ${PATH_PREFIX}/LFGMgr.cpp
    ${PATH_PREFIX}/LFGMgr.hpp
    ${PATH_PREFIX}/LFGPlayerData.cpp
    ${PATH_PREFIX}/LFGPlayerData.hpp
)

source_group(Management\\LFG FILES ${SRC_MANAGEMENT_LFG_FILES})
unset(PATH_PREFIX)
