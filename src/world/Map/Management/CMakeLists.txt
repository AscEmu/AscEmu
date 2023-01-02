# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Map/Management)

set(SRC_MAP_MANAGEMENT_FILES
    ${PATH_PREFIX}/InstanceNumberGen.hpp
    ${PATH_PREFIX}/MapManagementGlobals.hpp
    ${PATH_PREFIX}/MapMgr.cpp
    ${PATH_PREFIX}/MapMgr.hpp
    ${PATH_PREFIX}/MapMgrDefines.hpp
)

source_group(Map\\Management FILES ${SRC_MAP_MANAGEMENT_FILES})
unset(PATH_PREFIX)
