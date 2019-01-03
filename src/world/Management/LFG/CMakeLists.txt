# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Management/LFG)

set(SRC_MANAGEMENT_LFG_FILES
   ${PATH_PREFIX}/LFG.h
   ${PATH_PREFIX}/LFGGroupData.cpp
   ${PATH_PREFIX}/LFGGroupData.h
   ${PATH_PREFIX}/LFGMgr.cpp
   ${PATH_PREFIX}/LFGMgr.h
   ${PATH_PREFIX}/LFGPlayerData.cpp
   ${PATH_PREFIX}/LFGPlayerData.h
)

source_group(Management\\LFG FILES ${SRC_MANAGEMENT_LFG_FILES})
unset(PATH_PREFIX)
